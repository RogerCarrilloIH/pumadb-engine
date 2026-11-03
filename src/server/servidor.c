/**
 * @file servidor.c
 * @brief Servidor concurrente de base de datos PumaDB sobre sockets TCP/IP.
 * 
 * Implementa un servidor concurrente multisesión en C para Linux:
 * 1. Socket de escucha TCP con bind() y listen().
 * 2. Manejador de señal SIGCHLD con sigaction() y waitpid() asíncrono para recolectar procesos hijos.
 * 3. Bucle concurrente con fork(): el proceso padre escucha y el proceso hijo atiende cada cliente.
 * 4. Procesamiento de consultas SQL delegadas a parser.c y db_engine.c.
 * 5. Soporte de puerto configurable por argumento de línea de comandos.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "protocol.h"
#include "utils.h"
#include "parser.h"
#include "banner.h"

/**
 * Manejador de la señal SIGCHLD.
 * Cuando un proceso hijo termina su ejecución tras atender al cliente, el sistema operativo
 * envía la señal SIGCHLD al padre. Este manejador invoca waitpid() en un bucle no bloqueante
 * (WNOHANG) para cosechar el estado de salida y evitar la acumulación de procesos 'zombie'.
 */
static void sigchld_handler(int sig) {
    (void)sig;
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        /* Se limpian los procesos hijos terminados */
    }
    errno = saved_errno;
}

/**
 * Registra formalmente el manejador de la señal SIGCHLD mediante la interfaz POSIX sigaction.
 * Utiliza SA_RESTART para que llamadas al sistema interrumpidas (como accept) se reinicien automáticamente.
 */
static void setup_sigchld_handler(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error al configurar sigaction para SIGCHLD");
        exit(EXIT_FAILURE);
    }
}

/**
 * Atiende la sesión de un cliente conectado. Esta función es ejecutada exclusivamente por el proceso
 * hijo generado mediante fork().
 * 
 * @param client_fd Descriptor del socket del cliente conectado.
 * @param client_ip Dirección IP en formato textual del cliente remoto.
 * @param client_port Puerto efímero de conexión del cliente.
 */
static void handle_client_session(int client_fd, const char *client_ip, int client_port) {
    char rx_buffer[BUFFER_SIZE];
    char tx_buffer[BUFFER_SIZE];

    printf("[Hijo PID %d] Sesion iniciada para cliente %s:%d\n", getpid(), client_ip, client_port);

    while (1) {
        /* Leer comando enviado por el cliente */
        ssize_t bytes_read = receive_message(client_fd, rx_buffer, sizeof(rx_buffer));
        if (bytes_read <= 0) {
            printf("[Hijo PID %d] Cliente %s:%d cerro la conexion.\n", getpid(), client_ip, client_port);
            break;
        }

        /* Ejecutar la sentencia SQL a través del parser */
        int parse_res = execute_sql_query(rx_buffer, tx_buffer, sizeof(tx_buffer));

        /* Si el cliente envió SALIR, notificar fin de sesión y terminar */
        if (parse_res == PARSE_DISCONNECT) {
            send_message(client_fd, MSG_DISCONNECT);
            printf("[Hijo PID %d] Cliente %s:%d solicito SALIR. Cerrando sesion.\n", getpid(), client_ip, client_port);
            break;
        }

        /* Enviar resultado de la consulta SQL al cliente */
        if (send_message(client_fd, tx_buffer) < 0) {
            perror("[Hijo] Error al transmitir respuesta por el socket");
            break;
        }
    }

    close(client_fd);
    printf("[Hijo PID %d] Sesion concluida exitosamente.\n", getpid());
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    /* Analizar argumentos de línea de comandos: ./bin/servidor [puerto] */
    if (argc >= 2) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Uso: %s [puerto]\n", argv[0]);
            fprintf(stderr, "Error: Puerto '%s' invalido. Debe encontrarse entre 1 y 65535.\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    /* Configurar recolector asíncrono de procesos hijos */
    setup_sigchld_handler();

    /* Paso 1: Creación del socket TCP (dominio IPv4, orientado a conexión) */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error al crear socket del servidor (socket)");
        return EXIT_FAILURE;
    }

    /* Paso 2: Permitir reutilización inmediata de la dirección y puerto (evita TIME_WAIT) */
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error en setsockopt(SO_REUSEADDR)");
        close(server_fd);
        return EXIT_FAILURE;
    }

    /* Paso 3: Asignar dirección IP y puerto local (bind) */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Escuchar en todas las interfaces de red */
    server_addr.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al enlazar socket a la interfaz y puerto (bind)");
        close(server_fd);
        return EXIT_FAILURE;
    }

    /* Paso 4: Poner el socket en modo de escucha pasiva (listen) */
    if (listen(server_fd, 10) < 0) {
        perror("Error al habilitar modo de escucha (listen)");
        close(server_fd);
        return EXIT_FAILURE;
    }

    /* Mostrar banner visual informativo */
    print_server_banner(port);

    /* Paso 5: Bucle principal de concurrencia mediante fork() */
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        /* Esperar y aceptar una conexión entrante */
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) {
                /* Llamada al sistema interrumpida por la señal SIGCHLD; reintentar accept */
                continue;
            }
            perror("Error al aceptar conexion cliente (accept)");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        int client_port = ntohs(client_addr.sin_port);

        printf("[Servidor Padre] Nueva conexion entrante recibida desde %s:%d\n", client_ip, client_port);

        /* Bifurcación del proceso para atención concurrente */
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error al bifurcar proceso con fork()");
            close(client_fd);
            continue;
        }

        if (pid == 0) {
            /* ========================================================
             * PROCESO HIJO
             * Cierra el socket maestro de escucha y procesa la sesión.
             * ======================================================== */
            close(server_fd);
            handle_client_session(client_fd, client_ip, client_port);
        } else {
            /* ========================================================
             * PROCESO PADRE
             * Cierra su descriptor del cliente y sigue esperando nuevas conexiones.
             * ======================================================== */
            close(client_fd);
        }
    }

    close(server_fd);
    return EXIT_SUCCESS;
}

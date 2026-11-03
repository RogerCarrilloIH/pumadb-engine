/**
 * @file cliente.c
 * @brief Cliente interactivo de consola para PumaDB Engine sobre sockets TCP/IP.
 * 
 * Permite conectarse de manera remota al servidor PumaDB:
 * 1. Configuración de IP remota y puerto mediante argumentos de línea de comandos.
 * 2. Conexión mediante sockets TCP estándar POSIX.
 * 3. Renderizado de banner visual en ASCII con colores ANSI estilo Copilot CLI.
 * 4. Bucle interactivo REPL para enviar sentencias SQL (SELECT, INSERT, UPDATE, DELETE).
 * 5. Desconexión controlada mediante el comando SALIR o EOF (Ctrl+D).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "protocol.h"
#include "utils.h"
#include "banner.h"

int main(int argc, char *argv[]) {
    const char *server_ip = DEFAULT_IP;
    int port = DEFAULT_PORT;

    /* Analizar argumentos de línea de comandos: ./bin/cliente [ip_servidor] [puerto] */
    if (argc >= 2) {
        server_ip = argv[1];
    }
    if (argc >= 3) {
        port = atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Uso: %s [ip_servidor] [puerto]\n", argv[0]);
            fprintf(stderr, "Error: Puerto '%s' invalido. Debe encontrarse entre 1 y 65535.\n", argv[2]);
            return EXIT_FAILURE;
        }
    }

    /* Paso 1: Creación del socket TCP del cliente */
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Error al crear socket cliente");
        return EXIT_FAILURE;
    }

    /* Paso 2: Configurar estructura con la IP y puerto del servidor remoto */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Error: Direccion IP '%s' invalida o no soportada.\n", server_ip);
        close(sock_fd);
        return EXIT_FAILURE;
    }

    /* Paso 3: Establecer conexión con el servidor (connect) */
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Error al conectar con PumaDB Server en %s:%d (servidor no disponible o conexion rechazada).\n", server_ip, port);
        close(sock_fd);
        return EXIT_FAILURE;
    }

    /* Paso 4: Desplegar interfaz gráfica ASCII estilo Copilot CLI */
    print_client_banner(server_ip, port);

    char input_line[MAX_LINE_LEN];
    char rx_buffer[BUFFER_SIZE];

    /* Paso 5: Bucle interactivo de lectura, transmisión y presentación (REPL) */
    while (1) {
        printf("\033[1;36m>\033[0m ");
        fflush(stdout);

        if (!fgets(input_line, sizeof(input_line), stdin)) {
            /* Manejo de cierre de entrada por combinación de teclas (Ctrl+D) */
            printf("\nDetectado fin de entrada (EOF). Cerrando sesion...\n");
            send_message(sock_fd, CMD_EXIT);
            break;
        }

        char *trimmed = trim_whitespace(input_line);
        if (strlen(trimmed) == 0) {
            continue;
        }

        /* Transmitir la consulta al servidor por el socket */
        if (send_message(sock_fd, trimmed) < 0) {
            fprintf(stderr, "Error al enviar comando al servidor.\n");
            break;
        }

        /* Si el comando fue SALIR, terminar la sesión local */
        if (str_case_equals(trimmed, CMD_EXIT)) {
            printf("\nDesconectando de PumaDB Server... ¡Hasta pronto!\n");
            break;
        }

        /* Esperar y leer la respuesta enviada por el motor de base de datos */
        ssize_t bytes = receive_message(sock_fd, rx_buffer, sizeof(rx_buffer));
        if (bytes <= 0) {
            printf("\nEl servidor finalizo la conexion.\n");
            break;
        }

        if (strcmp(rx_buffer, MSG_DISCONNECT) == 0) {
            printf("\nSesion finalizada por el servidor.\n");
            break;
        }

        /* Imprimir la respuesta devuelta por el servidor */
        printf("%s", rx_buffer);
        if (rx_buffer[strlen(rx_buffer) - 1] != '\n') {
            printf("\n");
        }
        printf("\n");
    }

    /* Paso 6: Cerrar descriptor de socket */
    close(sock_fd);
    return EXIT_SUCCESS;
}

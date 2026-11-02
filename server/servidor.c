/**
 * ============================================================================
 * SISTEMA MANEJADOR DE BASES DE DATOS (DBMS)
 * Arquitectura Cliente-Servidor con Sockets TCP/IP
 * Archivo: cliente.c
 * Entorno: Linux (POSIX compliant)
 * Lenguaje: C
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 16384 // Tamaño del buffer para recibir las respuestas del servidor

int main(int argc, char *argv[]) {
    // Validar que el usuario proporcione la IP del servidor y el puerto por argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso correcto: %s <IP_Servidor> <Puerto_TCP>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s 127.0.0.1 8080\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip_servidor = argv[1];
    int puerto = atoi(argv[2]);

    // 1. Crear el socket TCP/IP del lado del cliente (AF_INET para IPv4, SOCK_STREAM para TCP)
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error al crear el socket en el cliente");
        exit(EXIT_FAILURE);
    }

    // 2. Configurar la estructura sockaddr_in con los datos del servidor remoto
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(puerto);

    // Convertir la dirección IP de texto a formato binario de red
    if (inet_pton(AF_INET, ip_servidor, &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Error: Dirección IP inválida o no soportada (%s)\n", ip_servidor);
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 3. Conectarse al servidor remoto mediante la llamada al sistema connect()
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error al conectar con el servidor DBMS (¿Está encendido?)");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("================================================================\n");
    printf(" Conectado exitosamente al servidor DBMS en %s:%d\n", ip_servidor, puerto);
    printf(" Escribe tus comandos SQL (ej. SELECT * FROM Alumno) o 'SALIR'.\n");
    printf("================================================================\n\n");

    char comando[1024];
    char *respuesta = malloc(BUFFER_SIZE);
    if (!respuesta) {
        perror("Error al asignar memoria para la respuesta");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 4. Bucle interactivo para enviar comandos y recibir resultados
    while (1) {
        printf("SQL> ");
        fflush(stdout);

        // Leer la línea introducida por el usuario en la consola
        if (!fgets(comando, sizeof(comando), stdin)) {
            break;
        }

        // Limpiar el salto de línea al final de la cadena obtenida por teclado
        comando[strcspn(comando, "\r\n")] = '\0';
        
        // Si el usuario presiona Enter sin escribir nada, repetir el prompt
        if (strlen(comando) == 0) continue;

        // Enviar el comando SQL al servidor a través del socket TCP
        if (send(sock, comando, strlen(comando), 0) < 0) {
            perror("Error al enviar el comando al servidor");
            break;
        }

        // Recibir la respuesta enviada por el servidor
        memset(respuesta, 0, BUFFER_SIZE);
        ssize_t bytes_recibidos = recv(sock, respuesta, BUFFER_SIZE - 1, 0);
        if (bytes_recibidos <= 0) {
            printf("\n[CLIENTE] El servidor cerró la conexión abruptamente.\n");
            break;
        }

        // Imprimir en pantalla la respuesta devuelta por el motor de base de datos
        printf("%s\n", respuesta);

        // Si el comando enviado fue SALIR, terminamos la ejecución del cliente
        if (strcasecmp(comando, "SALIR") == 0 || strcasecmp(comando, "SALIR;") == 0) {
            break;
        }
    }

    // 5. Liberar recursos y cerrar el descriptor del socket
    free(respuesta);
    close(sock);
    printf("[CLIENTE] Sesión finalizada.\n");
    return 0;
}
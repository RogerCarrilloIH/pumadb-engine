/**
 * @file utils.c
 * @brief Implementación de funciones utilitarias y de red para PumaDB Engine.
 * 
 * Contiene funciones para manipulación y saneamiento de cadenas, comparaciones
 * insensibles a mayúsculas/minúsculas y transmisión robusta por sockets TCP.
 */

#include "utils.h"
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

/**
 * Elimina espacios en blanco, tabulaciones y saltos de línea (\r, \n)
 * tanto al principio como al final de la cadena de texto.
 * Opera "in-place" modificando la cadena original.
 */
char *trim_whitespace(char *str) {
    if (!str) return NULL;

    /* Avanzar puntero sobre los espacios iniciales */
    while (*str && isspace((unsigned char)*str)) {
        str++;
    }

    /* Si la cadena solo contenía espacios */
    if (*str == '\0') {
        return str;
    }

    /* Retroceder desde el último carácter eliminando espacios finales */
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';

    return str;
}

/**
 * Compara dos cadenas de texto ignorando la diferencia entre mayúsculas y minúsculas.
 * Retorna 1 si son equivalentes, o 0 si difieren.
 */
int str_case_equals(const char *s1, const char *s2) {
    if (!s1 || !s2) return 0;
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return 0;
        }
        s1++;
        s2++;
    }
    return (*s1 == '\0' && *s2 == '\0');
}

/**
 * Comprueba si la cadena comienza con un prefijo específico, ignorando mayúsculas/minúsculas.
 * Retorna 1 si coincide el prefijo, o 0 en caso contrario.
 */
int str_starts_with_case(const char *str, const char *prefix) {
    if (!str || !prefix) return 0;
    while (*prefix) {
        if (tolower((unsigned char)*str) != tolower((unsigned char)*prefix)) {
            return 0;
        }
        str++;
        prefix++;
    }
    return 1;
}

/**
 * Envía un mensaje completo por un descriptor de socket TCP.
 * Dado que write() puede transmitir menos bytes de los solicitados por fragmentación TCP,
 * esta función itera en bucle hasta transmitir el mensaje completo.
 */
ssize_t send_message(int socket_fd, const char *msg) {
    if (!msg || socket_fd < 0) return -1;
    size_t total_len = strlen(msg);
    size_t bytes_sent = 0;

    while (bytes_sent < total_len) {
        ssize_t res = write(socket_fd, msg + bytes_sent, total_len - bytes_sent);
        if (res <= 0) {
            return -1; /* Error de transmisión o socket cerrado */
        }
        bytes_sent += (size_t)res;
    }
    return (ssize_t)bytes_sent;
}

/**
 * Lee un mensaje desde el socket TCP hacia el búfer indicado.
 * Limpia el búfer con ceros y asegura terminación nula '\0' al final de los bytes leídos.
 */
ssize_t receive_message(int socket_fd, char *buffer, size_t max_len) {
    if (!buffer || max_len == 0 || socket_fd < 0) return -1;
    memset(buffer, 0, max_len);

    ssize_t bytes_read = read(socket_fd, buffer, max_len - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
    }
    return bytes_read;
}

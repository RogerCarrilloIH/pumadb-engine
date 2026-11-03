#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <sys/types.h>

/**
 * Elimina espacios en blanco, tabs y saltos de línea al inicio y al final de una cadena.
 * Modifica la cadena en su lugar y retorna el puntero al inicio resultante.
 */
char *trim_whitespace(char *str);

/**
 * Comparación de cadenas sin distinguir entre mayúsculas y minúsculas (estilo strcasecmp).
 * Retorna 1 si son iguales, 0 en caso contrario.
 */
int str_case_equals(const char *s1, const char *s2);

/**
 * Comprueba si la cadena comienza con un prefijo dado, ignorando mayúsculas/minúsculas.
 * Retorna 1 si coincide, 0 en caso contrario.
 */
int str_starts_with_case(const char *str, const char *prefix);

/**
 * Envía una cadena completa a través de un descriptor de socket TCP.
 * Garantiza la transmisión de todos los bytes requeridos.
 * Retorna el número de bytes enviados o -1 en caso de error.
 */
ssize_t send_message(int socket_fd, const char *msg);

/**
 * Recibe un mensaje a través del socket TCP en el búfer proporcionado y lo termina con '\0'.
 * Retorna el número de bytes leídos, 0 si la conexión fue cerrada por el par, o -1 en error.
 */
ssize_t receive_message(int socket_fd, char *buffer, size_t max_len);

#endif /* UTILS_H */


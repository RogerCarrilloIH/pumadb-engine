#ifndef BANNER_H
#define BANNER_H

/**
 * Imprime el banner estilo Copilot CLI para el cliente PumaDB,
 * mostrando el logo ASCII del puma, colores ANSI y el estado de la conexión.
 */
void print_client_banner(const char *server_ip, int port);

/**
 * Imprime el banner estilo Copilot CLI para el servidor PumaDB,
 * mostrando el puerto de escucha y el modo concurrente activo.
 */
void print_server_banner(int port);

#endif /* BANNER_H */


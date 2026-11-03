#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

/* Códigos de retorno del procesador de comandos */
#define PARSE_SUCCESS 0
#define PARSE_DISCONNECT 1
#define PARSE_ERROR -1

/**
 * Analiza sintácticamente y ejecuta la consulta SQL proporcionada,
 * direccionando la llamada a las funciones correspondientes de db_engine.
 * 
 * Si el comando es "SALIR", almacena MSG_DISCONNECT y retorna PARSE_DISCONNECT.
 * Para cualquier otra consulta, coloca la respuesta en response_buf y retorna PARSE_SUCCESS
 * o PARSE_ERROR según el resultado de la operación.
 */
int execute_sql_query(const char *raw_query, char *response_buf, size_t max_len);

#endif /* PARSER_H */


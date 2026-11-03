#ifndef PROTOCOL_H
#define PROTOCOL_H

/* Puerto e IP por defecto */
#define DEFAULT_PORT 8080
#define DEFAULT_IP "127.0.0.1"

/* Tamaños de búfer y límites del sistema */
#define BUFFER_SIZE 4096
#define MAX_LINE_LEN 1024
#define MAX_FIELDS 10
#define MAX_COL_NAME_LEN 64
#define MAX_VALUE_LEN 256

/* Ruta predeterminada del archivo de la tabla Alumno */
#define DB_FILE_PATH "data/alumno.txt"
#define DB_TMP_FILE_PATH "data/alumno.tmp"

/* Comandos reconocidos */
#define CMD_EXIT "SALIR"
#define CMD_SELECT "SELECT"
#define CMD_INSERT "INSERT"
#define CMD_UPDATE "UPDATE"
#define CMD_DELETE "DELETE"

/* Respuestas estándar del servidor */
#define MSG_OK_INSERT "OK: Registro insertado exitosamente.\n"
#define MSG_OK_UPDATE "OK: Registro actualizado exitosamente.\n"
#define MSG_OK_DELETE "OK: Registro eliminado exitosamente.\n"
#define MSG_ERR_SYNTAX "ERROR: Sintaxis de comando no reconocida o invalida.\n"
#define MSG_ERR_NOT_FOUND "ERROR: Registro no encontrado con la condicion dada.\n"
#define MSG_ERR_ID_UPDATE "ERROR: La columna Id es llave primaria y no puede ser modificada.\n"
#define MSG_ERR_FILE "ERROR: No se pudo acceder o procesar el archivo de datos.\n"
#define MSG_DISCONNECT "DISCONNECT\n"

#endif /* PROTOCOL_H */


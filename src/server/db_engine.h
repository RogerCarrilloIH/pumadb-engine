#ifndef DB_ENGINE_H
#define DB_ENGINE_H

#include <stddef.h>

/**
 * Estructura que representa un registro de la tabla Alumno.
 */
typedef struct {
    int id;
    char nombre[64];
    char apellido[64];
    int semestre;
    char carrera[64];
} AlumnoRecord;

/**
 * Ejecuta SELECT * FROM Alumno.
 * Lee todos los registros del archivo alumno.txt con bloqueo compartido (LOCK_SH)
 * y almacena el resultado en response_buf.
 * Retorna 0 en éxito, -1 en caso de error.
 */
int db_select_all(char *response_buf, size_t max_len);

/**
 * Ejecuta INSERT INTO Alumno VALUES (nombre, apellido, semestre, carrera).
 * Calcula automáticamente el siguiente Id (max Id + 1) con bloqueo exclusivo (LOCK_EX)
 * y agrega el registro al final del archivo.
 * Retorna 0 en éxito, -1 en caso de error.
 */
int db_insert(const char *nombre, const char *apellido, int semestre, const char *carrera, char *response_buf, size_t max_len);

/**
 * Ejecuta UPDATE Alumno SET columna=nuevo_valor WHERE Id=target_id.
 * Modifica atómicamente el registro coincidente en alumno.txt.
 * No permite modificar la columna 'Id'.
 * Retorna 0 en éxito, -1 en caso de error o registro no encontrado.
 */
int db_update(int target_id, const char *column, const char *new_value, char *response_buf, size_t max_len);

/**
 * Ejecuta DELETE FROM Alumno WHERE Id=target_id.
 * Elimina atómicamente el registro coincidente de alumno.txt usando un archivo temporal.
 * Retorna 0 en éxito, -1 en caso de error o registro no encontrado.
 */
int db_delete(int target_id, char *response_buf, size_t max_len);

#endif /* DB_ENGINE_H */


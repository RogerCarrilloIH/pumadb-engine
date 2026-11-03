/**
 * @file db_engine.c
 * @brief Motor de persistencia y operaciones CRUD sobre archivos planos de PumaDB.
 * 
 * Implementa el manejo de la tabla Alumno (alumno.txt) cumpliendo con:
 * - Persistencia en archivo de texto plano con campos separados por coma (,).
 * - Llave primaria 'Id' numérica, autoincremental y de solo lectura.
 * - Concurrencia segura mediante bloqueos de archivo (flock) a nivel de kernel.
 * - Atomicidad en operaciones de actualización y eliminación mediante archivos temporales y rename().
 */

#include "db_engine.h"
#include "protocol.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

/* Encabezado esperado de la tabla Alumno */
static const char *HEADER_EXPECTED = "Id,Nombre,Apellido,Semestre,Carrera";

/**
 * Consulta y devuelve todos los registros de la tabla Alumno.
 * 
 * Procedimiento:
 * 1. Abre el archivo alumno.txt en modo lectura ("r").
 * 2. Aplica un bloqueo compartido LOCK_SH para permitir lecturas concurrentes sin interferir con escrituras.
 * 3. Lee secuencialmente las líneas del archivo y las concatena en el búfer de respuesta.
 * 4. Libera el bloqueo (LOCK_UN) y cierra el descriptor de archivo.
 */
int db_select_all(char *response_buf, size_t max_len) {
    if (!response_buf || max_len == 0) return -1;
    response_buf[0] = '\0';

    FILE *fp = fopen(DB_FILE_PATH, "r");
    if (!fp) {
        snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
        return -1;
    }

    /* Adquirir bloqueo compartido de lectura (LOCK_SH) */
    if (flock(fileno(fp), LOCK_SH) != 0) {
        fclose(fp);
        snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
        return -1;
    }

    char line[MAX_LINE_LEN];
    size_t current_len = 0;

    /* Leer cada renglón del archivo y acumularlo en response_buf */
    while (fgets(line, sizeof(line), fp)) {
        size_t line_len = strlen(line);
        if (current_len + line_len + 1 < max_len) {
            strcat(response_buf, line);
            current_len += line_len;
        } else {
            /* Límite de capacidad alcanzado */
            break;
        }
    }

    /* Liberar bloqueo y cerrar archivo */
    flock(fileno(fp), LOCK_UN);
    fclose(fp);
    return 0;
}

/**
 * Inserta un nuevo alumno en la tabla con un ID autoincremental calculado.
 * 
 * Procedimiento:
 * 1. Abre el archivo en modo "r+" (o "w+" si no existe para inicializar la cabecera).
 * 2. Aplica un bloqueo exclusivo LOCK_EX para asegurar que ningún otro proceso inserte o modifique al mismo tiempo.
 * 3. Lee el archivo para encontrar el valor máximo de Id registrado.
 * 4. Asigna nuevo_id = max_id + 1.
 * 5. Se posiciona al final del archivo (SEEK_END) y escribe el nuevo registro.
 * 6. Vuelca el búfer con fflush(), libera el bloqueo exclusivo y cierra el archivo.
 */
int db_insert(const char *nombre, const char *apellido, int semestre, const char *carrera, char *response_buf, size_t max_len) {
    if (!nombre || !apellido || !carrera || !response_buf) return -1;

    FILE *fp = fopen(DB_FILE_PATH, "r+");
    if (!fp) {
        /* Si el archivo aún no existe, se crea con su encabezado estándar */
        fp = fopen(DB_FILE_PATH, "w+");
        if (!fp) {
            snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
            return -1;
        }
        fprintf(fp, "%s\n", HEADER_EXPECTED);
        fflush(fp);
    }

    /* Adquirir bloqueo exclusivo para escritura (LOCK_EX) */
    if (flock(fileno(fp), LOCK_EX) != 0) {
        fclose(fp);
        snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
        return -1;
    }

    /* Calcular el ID autoincremental buscando el valor máximo existente */
    rewind(fp);
    char line[MAX_LINE_LEN];
    int max_id = 0;
    int line_num = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        if (line_num == 1) continue; /* Omitir renglón de encabezado */

        char *trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0) continue;

        int curr_id = 0;
        if (sscanf(trimmed, "%d,", &curr_id) == 1) {
            if (curr_id > max_id) {
                max_id = curr_id;
            }
        }
    }

    int new_id = max_id + 1;

    /* Posicionarse al final del archivo para anexar el registro */
    fseek(fp, 0, SEEK_END);
    fprintf(fp, "%d,%s,%s,%d,%s\n", new_id, nombre, apellido, semestre, carrera);
    fflush(fp);

    /* Liberar bloqueo y cerrar archivo */
    flock(fileno(fp), LOCK_UN);
    fclose(fp);

    snprintf(response_buf, max_len, "%s", MSG_OK_INSERT);
    return 0;
}

/**
 * Actualiza una columna específica del alumno identificado por target_id.
 * 
 * Procedimiento:
 * 1. Valida que la columna a modificar no sea 'Id' (la llave primaria es inmutable según las especificaciones).
 * 2. Abre el archivo original con bloqueo exclusivo LOCK_EX y abre un archivo temporal (alumno.tmp).
 * 3. Copia el encabezado al archivo temporal.
 * 4. Lee línea por línea: si el Id coincide con target_id, sustituye el campo solicitado; si no, copia la línea tal cual.
 * 5. Si se encontró el registro, renombra atómicamente alumno.tmp sobre alumno.txt con rename().
 * 6. Si no se encontró, descarta el archivo temporal y responde error.
 */
int db_update(int target_id, const char *column, const char *new_value, char *response_buf, size_t max_len) {
    if (!column || !new_value || !response_buf) return -1;

    /* Validación estricta: La llave primaria Id no puede modificarse con UPDATE */
    if (str_case_equals(column, "Id")) {
        snprintf(response_buf, max_len, "%s", MSG_ERR_ID_UPDATE);
        return -1;
    }

    /* Determinar el índice de columna a modificar (1: Nombre, 2: Apellido, 3: Semestre, 4: Carrera) */
    int col_index = -1;
    if (str_case_equals(column, "Nombre")) col_index = 1;
    else if (str_case_equals(column, "Apellido")) col_index = 2;
    else if (str_case_equals(column, "Semestre")) col_index = 3;
    else if (str_case_equals(column, "Carrera")) col_index = 4;

    if (col_index == -1) {
        snprintf(response_buf, max_len, "ERROR: La columna '%s' no existe en la tabla Alumno.\n", column);
        return -1;
    }

    FILE *fp_in = fopen(DB_FILE_PATH, "r");
    if (!fp_in) {
        snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
        return -1;
    }

    /* Bloqueo exclusivo sobre el archivo fuente */
    if (flock(fileno(fp_in), LOCK_EX) != 0) {
        fclose(fp_in);
        snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
        return -1;
    }

    FILE *fp_out = fopen(DB_TMP_FILE_PATH, "w");
    if (!fp_out) {
        flock(fileno(fp_in), LOCK_UN);
        fclose(fp_in);
        snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
        return -1;
    }

    char line[MAX_LINE_LEN];
    int line_num = 0;
    int found = 0;

    while (fgets(line, sizeof(line), fp_in)) {
        line_num++;
        if (line_num == 1) {
            /* Preservar la cabecera intacta */
            fputs(line, fp_out);
            continue;
        }

        char line_copy[MAX_LINE_LEN];
        strncpy(line_copy, line, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';
        char *trimmed = trim_whitespace(line_copy);
        if (strlen(trimmed) == 0) continue;

        int row_id = 0;
        char row_nom[64] = {0};
        char row_ape[64] = {0};
        int row_sem = 0;
        char row_car[64] = {0};

        if (sscanf(trimmed, "%d,%63[^,],%63[^,],%d,%63s", &row_id, row_nom, row_ape, &row_sem, row_car) == 5) {
            if (row_id == target_id) {
                found = 1;
                /* Actualizar la columna correspondiente */
                if (col_index == 1) {
                    strncpy(row_nom, new_value, sizeof(row_nom) - 1);
                } else if (col_index == 2) {
                    strncpy(row_ape, new_value, sizeof(row_ape) - 1);
                } else if (col_index == 3) {
                    row_sem = atoi(new_value);
                } else if (col_index == 4) {
                    strncpy(row_car, new_value, sizeof(row_car) - 1);
                }
                fprintf(fp_out, "%d,%s,%s,%d,%s\n", row_id, row_nom, row_ape, row_sem, row_car);
            } else {
                /* Copiar registro sin modificación */
                fputs(line, fp_out);
            }
        } else {
            fputs(line, fp_out);
        }
    }

    fflush(fp_out);
    flock(fileno(fp_in), LOCK_UN);
    fclose(fp_in);
    fclose(fp_out);

    /* Sustitución atómica */
    if (found) {
        if (rename(DB_TMP_FILE_PATH, DB_FILE_PATH) == 0) {
            snprintf(response_buf, max_len, "%s", MSG_OK_UPDATE);
            return 0;
        } else {
            unlink(DB_TMP_FILE_PATH);
            snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
            return -1;
        }
    } else {
        unlink(DB_TMP_FILE_PATH);
        snprintf(response_buf, max_len, "%s", MSG_ERR_NOT_FOUND);
        return -1;
    }
}

/**
 * Elimina un registro de la tabla Alumno según su Id.
 * 
 * Procedimiento:
 * 1. Abre alumno.txt con bloqueo exclusivo LOCK_EX y abre el archivo temporal alumno.tmp.
 * 2. Escribe la cabecera en el archivo temporal.
 * 3. Lee cada línea: si coincide con target_id, la omite (eliminación); de lo contrario, la escribe en alumno.tmp.
 * 4. Si se encontró y eliminó el registro, renombra alumno.tmp a alumno.txt atómicamente.
 * 5. Si no se encontró el Id, borra alumno.tmp y retorna error indicando que no existe.
 */
int db_delete(int target_id, char *response_buf, size_t max_len) {
    if (!response_buf) return -1;

    FILE *fp_in = fopen(DB_FILE_PATH, "r");
    if (!fp_in) {
        snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
        return -1;
    }

    /* Bloqueo exclusivo sobre el archivo fuente */
    if (flock(fileno(fp_in), LOCK_EX) != 0) {
        fclose(fp_in);
        snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
        return -1;
    }

    FILE *fp_out = fopen(DB_TMP_FILE_PATH, "w");
    if (!fp_out) {
        flock(fileno(fp_in), LOCK_UN);
        fclose(fp_in);
        snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
        return -1;
    }

    char line[MAX_LINE_LEN];
    int line_num = 0;
    int found = 0;

    while (fgets(line, sizeof(line), fp_in)) {
        line_num++;
        if (line_num == 1) {
            /* Copiar la cabecera */
            fputs(line, fp_out);
            continue;
        }

        char line_copy[MAX_LINE_LEN];
        strncpy(line_copy, line, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';
        char *trimmed = trim_whitespace(line_copy);
        if (strlen(trimmed) == 0) continue;

        int row_id = 0;
        if (sscanf(trimmed, "%d,", &row_id) == 1) {
            if (row_id == target_id) {
                found = 1;
                /* Omitir este renglón para efectuar el borrado */
                continue;
            }
        }
        fputs(line, fp_out);
    }

    fflush(fp_out);
    flock(fileno(fp_in), LOCK_UN);
    fclose(fp_in);
    fclose(fp_out);

    /* Sustitución atómica */
    if (found) {
        if (rename(DB_TMP_FILE_PATH, DB_FILE_PATH) == 0) {
            snprintf(response_buf, max_len, "%s", MSG_OK_DELETE);
            return 0;
        } else {
            unlink(DB_TMP_FILE_PATH);
            snprintf(response_buf, max_len, "%s", MSG_ERR_FILE);
            return -1;
        }
    } else {
        unlink(DB_TMP_FILE_PATH);
        snprintf(response_buf, max_len, "%s", MSG_ERR_NOT_FOUND);
        return -1;
    }
}

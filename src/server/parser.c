/**
 * @file parser.c
 * @brief Intérprete y analizador léxico/sintáctico de sentencias SQL para PumaDB.
 * 
 * Satisface los requerimientos del examen de Arquitecturas Cliente-Servidor:
 * 1. SELECT * FROM Alumno (con o sin punto y coma final).
 * 2. INSERT INTO Alumno VALUES (Nombre,Apellido,Semestre,Carrera) con cadenas sin comillas.
 * 3. UPDATE Alumno SET Columna=Valor WHERE Id=X (rechaza modificaciones sobre la columna Id).
 * 4. DELETE FROM Alumno WHERE Id=X.
 * 5. SALIR para desconexión limpia del cliente.
 */

#include "parser.h"
#include "db_engine.h"
#include "protocol.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Busca la primera ocurrencia de needle dentro de haystack sin distinguir mayúsculas/minúsculas.
 * Retorna un puntero al inicio de la coincidencia o NULL si no existe.
 */
static char *find_substr_case(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return (char *)haystack;

    while (*haystack) {
        if (str_starts_with_case(haystack, needle)) {
            return (char *)haystack;
        }
        haystack++;
    }
    return NULL;
}

/**
 * Analiza la cadena cruda de entrada y ejecuta la operación SQL correspondiente.
 */
int execute_sql_query(const char *raw_query, char *response_buf, size_t max_len) {
    if (!raw_query || !response_buf || max_len == 0) return PARSE_ERROR;
    response_buf[0] = '\0';

    /* Copia de trabajo para aislar y procesar la cadena sin alterar memoria externa */
    char query[MAX_LINE_LEN];
    strncpy(query, raw_query, sizeof(query) - 1);
    query[sizeof(query) - 1] = '\0';

    /* Saneamiento de espacios en blanco */
    char *trimmed = trim_whitespace(query);

    /* En SQL el ';' final es opcional según la especificación del examen */
    size_t len = strlen(trimmed);
    if (len > 0 && trimmed[len - 1] == ';') {
        trimmed[len - 1] = '\0';
        trimmed = trim_whitespace(trimmed);
        len = strlen(trimmed);
    }

    if (len == 0) {
        snprintf(response_buf, max_len, "%s", MSG_ERR_SYNTAX);
        return PARSE_ERROR;
    }

    /* -------------------------------------------------------------
     * COMANDO ESPECIAL: SALIR
     * ------------------------------------------------------------- */
    if (str_case_equals(trimmed, CMD_EXIT)) {
        snprintf(response_buf, max_len, "%s", MSG_DISCONNECT);
        return PARSE_DISCONNECT;
    }

    /* -------------------------------------------------------------
     * COMANDO 1: SELECT * FROM Alumno
     * ------------------------------------------------------------- */
    if (str_starts_with_case(trimmed, "SELECT")) {
        char *from_ptr = find_substr_case(trimmed, "FROM");
        if (!from_ptr) {
            snprintf(response_buf, max_len, "%s", MSG_ERR_SYNTAX);
            return PARSE_ERROR;
        }

        /* Validar que la tabla objetivo sea Alumno */
        char *table_ptr = trim_whitespace(from_ptr + 4);
        if (!str_case_equals(table_ptr, "Alumno")) {
            snprintf(response_buf, max_len, "ERROR: La tabla '%s' no existe. Tabla disponible: Alumno.\n", table_ptr);
            return PARSE_ERROR;
        }

        if (db_select_all(response_buf, max_len) == 0) {
            return PARSE_SUCCESS;
        } else {
            return PARSE_ERROR;
        }
    }

    /* -------------------------------------------------------------
     * COMANDO 2: INSERT INTO Alumno VALUES (Nombre,Apellido,Semestre,Carrera)
     * ------------------------------------------------------------- */
    if (str_starts_with_case(trimmed, "INSERT")) {
        char *into_ptr = find_substr_case(trimmed, "INTO");
        char *values_ptr = find_substr_case(trimmed, "VALUES");

        if (!into_ptr || !values_ptr || values_ptr <= into_ptr) {
            snprintf(response_buf, max_len, "%s", MSG_ERR_SYNTAX);
            return PARSE_ERROR;
        }

        /* Extraer nombre de la tabla entre INTO y VALUES */
        char table_name[64] = {0};
        char *tbl_start = trim_whitespace(into_ptr + 4);
        size_t tbl_len = (size_t)(values_ptr - tbl_start);
        if (tbl_len >= sizeof(table_name)) tbl_len = sizeof(table_name) - 1;
        strncpy(table_name, tbl_start, tbl_len);
        trim_whitespace(table_name);

        if (!str_case_equals(table_name, "Alumno")) {
            snprintf(response_buf, max_len, "ERROR: Tabla invalida '%s'. Tabla disponible: Alumno.\n", table_name);
            return PARSE_ERROR;
        }

        /* Extraer la tupla dentro de los paréntesis: (val1, val2, val3, val4) */
        char *open_paren = strchr(values_ptr, '(');
        char *close_paren = strrchr(values_ptr, ')');
        if (!open_paren || !close_paren || close_paren <= open_paren) {
            snprintf(response_buf, max_len, "%s", MSG_ERR_SYNTAX);
            return PARSE_ERROR;
        }

        *close_paren = '\0';
        char *args_str = open_paren + 1;

        /* Desglosar los 4 campos esperados */
        char val_nombre[64] = {0};
        char val_apellido[64] = {0};
        char val_semestre_str[32] = {0};
        char val_carrera[64] = {0};

        char *token = strtok(args_str, ",");
        if (token) strncpy(val_nombre, trim_whitespace(token), sizeof(val_nombre) - 1);

        token = strtok(NULL, ",");
        if (token) strncpy(val_apellido, trim_whitespace(token), sizeof(val_apellido) - 1);

        token = strtok(NULL, ",");
        if (token) strncpy(val_semestre_str, trim_whitespace(token), sizeof(val_semestre_str) - 1);

        token = strtok(NULL, ",");
        if (token) strncpy(val_carrera, trim_whitespace(token), sizeof(val_carrera) - 1);

        if (strlen(val_nombre) == 0 || strlen(val_apellido) == 0 ||
            strlen(val_semestre_str) == 0 || strlen(val_carrera) == 0) {
            snprintf(response_buf, max_len, "ERROR: Se requieren exactamente 4 campos: (Nombre,Apellido,Semestre,Carrera)\n");
            return PARSE_ERROR;
        }

        int semestre = atoi(val_semestre_str);
        if (db_insert(val_nombre, val_apellido, semestre, val_carrera, response_buf, max_len) == 0) {
            return PARSE_SUCCESS;
        } else {
            return PARSE_ERROR;
        }
    }

    /* -------------------------------------------------------------
     * COMANDO 3: UPDATE Alumno SET Columna=Valor WHERE Id=X
     * ------------------------------------------------------------- */
    if (str_starts_with_case(trimmed, "UPDATE")) {
        char *set_ptr = find_substr_case(trimmed, "SET");
        char *where_ptr = find_substr_case(trimmed, "WHERE");

        if (!set_ptr || !where_ptr || where_ptr <= set_ptr) {
            snprintf(response_buf, max_len, "%s", MSG_ERR_SYNTAX);
            return PARSE_ERROR;
        }

        /* Validar tabla objetivo */
        char table_name[64] = {0};
        char *tbl_start = trim_whitespace(trimmed + 6);
        size_t tbl_len = (size_t)(set_ptr - tbl_start);
        if (tbl_len >= sizeof(table_name)) tbl_len = sizeof(table_name) - 1;
        strncpy(table_name, tbl_start, tbl_len);
        trim_whitespace(table_name);

        if (!str_case_equals(table_name, "Alumno")) {
            snprintf(response_buf, max_len, "ERROR: Tabla invalida '%s'. Tabla disponible: Alumno.\n", table_name);
            return PARSE_ERROR;
        }

        /* Extraer cláusula SET (Columna=Valor) */
        char set_clause[128] = {0};
        char *set_start = set_ptr + 3;
        size_t set_len = (size_t)(where_ptr - set_start);
        if (set_len >= sizeof(set_clause)) set_len = sizeof(set_clause) - 1;
        strncpy(set_clause, set_start, set_len);
        trim_whitespace(set_clause);

        char *eq_set = strchr(set_clause, '=');
        if (!eq_set) {
            snprintf(response_buf, max_len, "%s", MSG_ERR_SYNTAX);
            return PARSE_ERROR;
        }
        *eq_set = '\0';
        char *col_name = trim_whitespace(set_clause);
        char *new_val = trim_whitespace(eq_set + 1);

        /* Extraer cláusula WHERE (Id=Valor) */
        char *where_clause = trim_whitespace(where_ptr + 5);
        char *eq_where = strchr(where_clause, '=');
        if (!eq_where) {
            snprintf(response_buf, max_len, "%s", MSG_ERR_SYNTAX);
            return PARSE_ERROR;
        }
        *eq_where = '\0';
        char *where_col = trim_whitespace(where_clause);
        char *where_val = trim_whitespace(eq_where + 1);

        if (!str_case_equals(where_col, "Id")) {
            snprintf(response_buf, max_len, "ERROR: La condicion WHERE solo admite la columna 'Id'.\n");
            return PARSE_ERROR;
        }

        int target_id = atoi(where_val);
        if (target_id <= 0) {
            snprintf(response_buf, max_len, "ERROR: Id invalido en la clausula WHERE.\n");
            return PARSE_ERROR;
        }

        if (db_update(target_id, col_name, new_val, response_buf, max_len) == 0) {
            return PARSE_SUCCESS;
        } else {
            return PARSE_ERROR;
        }
    }

    /* -------------------------------------------------------------
     * COMANDO 4: DELETE FROM Alumno WHERE Id=X
     * ------------------------------------------------------------- */
    if (str_starts_with_case(trimmed, "DELETE")) {
        char *from_ptr = find_substr_case(trimmed, "FROM");
        char *where_ptr = find_substr_case(trimmed, "WHERE");

        if (!from_ptr || !where_ptr || where_ptr <= from_ptr) {
            snprintf(response_buf, max_len, "%s", MSG_ERR_SYNTAX);
            return PARSE_ERROR;
        }

        /* Validar nombre de tabla */
        char table_name[64] = {0};
        char *tbl_start = trim_whitespace(from_ptr + 4);
        size_t tbl_len = (size_t)(where_ptr - tbl_start);
        if (tbl_len >= sizeof(table_name)) tbl_len = sizeof(table_name) - 1;
        strncpy(table_name, tbl_start, tbl_len);
        trim_whitespace(table_name);

        if (!str_case_equals(table_name, "Alumno")) {
            snprintf(response_buf, max_len, "ERROR: Tabla invalida '%s'. Tabla disponible: Alumno.\n", table_name);
            return PARSE_ERROR;
        }

        /* Extraer condición WHERE (Id=Valor) */
        char *where_clause = trim_whitespace(where_ptr + 5);
        char *eq_where = strchr(where_clause, '=');
        if (!eq_where) {
            snprintf(response_buf, max_len, "%s", MSG_ERR_SYNTAX);
            return PARSE_ERROR;
        }
        *eq_where = '\0';
        char *where_col = trim_whitespace(where_clause);
        char *where_val = trim_whitespace(eq_where + 1);

        if (!str_case_equals(where_col, "Id")) {
            snprintf(response_buf, max_len, "ERROR: La condicion WHERE solo admite la columna 'Id'.\n");
            return PARSE_ERROR;
        }

        int target_id = atoi(where_val);
        if (target_id <= 0) {
            snprintf(response_buf, max_len, "ERROR: Id invalido en la clausula WHERE.\n");
            return PARSE_ERROR;
        }

        if (db_delete(target_id, response_buf, max_len) == 0) {
            return PARSE_SUCCESS;
        } else {
            return PARSE_ERROR;
        }
    }

    /* Si no coincide con ninguna regla anterior */
    snprintf(response_buf, max_len, "%s", MSG_ERR_SYNTAX);
    return PARSE_ERROR;
}

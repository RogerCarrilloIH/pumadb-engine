 /**
 * ============================================================================
 * SISTEMA MANEJADOR DE BASES DE DATOS (DBMS)
 * Archivo: servidor.c
 * Entorno: Linux (POSIX compliant)
 * Lenguaje: C
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>


/* Constantes del motor */
#define DB_FILE "alumno.db"
#define MAGIC_NUMBER 0x4C4E5844 // Firma "LNXD" para validar integridad del archivo binario
#define DB_VERSION 1
#define BUFFER_SIZE 16384

/**
 * Superbloque de metadatos (64 bytes exactos al inicio de alumno.db).
 * Permite conocer el último ID autoincremental de forma instantánea en O(1).
 */
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t version;
    uint32_t ultimo_id;
    uint32_t total_activos;
    uint32_t total_borrados;
    uint64_t ultima_modif;
    uint8_t  reservado[36];
} DBHeader;

/* 
 * Estructura de cada registro de alumno (Exactamente 192 bytes en disco).
 * Simula los campos requeridos: Id, Nombre, Apellido, Semestre y Carrera[cite: 1, 6].
 */
typedef struct __attribute__((packed)) {
    int32_t id;
    int32_t semestre;
    uint8_t activo; // 1 = Activo (Vivo), 0 = Borrado lógico (Tombstone)
    uint8_t reservado[7];
    char    nombre[56];
    char    apellido[60];
    char    carrera[60];
} AlumnoRecord;

/* Variables globales para control de concurrencia y archivo */
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
int db_fd = -1;

/**
 * ============================================================================
 * FUNCIONES AUXILIARES E INICIALIZACIÓN DEL MOTOR BINARIO
 * ============================================================================
 */

/**
 * Limpia espacios en blanco, saltos de línea, comillas (' o ") 
 * y el punto y coma final (;) de cualquier comando o argumento recibido.
 */
void limpiar_token(char *str) {
    if (!str) return;
    int len = strlen(str);
    
    // Eliminar caracteres basura por la derecha
    while (len > 0 && (isspace((unsigned char)str[len - 1]) ||
                       str[len - 1] == ';' ||
                       str[len - 1] == '\'' ||
                       str[len - 1] == '"')) {
        str[--len] = '\0';
    }
    
    // Eliminar espacios o comillas iniciales por la izquierda
    int start = 0;
    while (str[start] != '\0' && (isspace((unsigned char)str[start]) ||
                                  str[start] == '\'' ||
                                  str[start] == '"')) {
        start++;
    }
    
    if (start > 0) {
        memmove(str, str + start, len - start + 1);
    }
}

/**
 * Inicializa el archivo de base de datos binaria. 
 * Si no existe, escribe el Superbloque inicial con valores en cero.
 */
void inicializar_motor_db() {
    // Abrir archivo con llamadas al sistema POSIX: lectura/escritura, creación si no existe (permisos 0644)
    db_fd = open(DB_FILE, O_RDWR | O_CREAT, 0644);
    if (db_fd < 0) {
        perror("Error crítico al abrir o crear el archivo de base de datos");
        exit(EXIT_FAILURE);
    }

    DBHeader header;
    ssize_t leidos = pread(db_fd, &header, sizeof(DBHeader), 0);

    // Si el archivo está vacío, inicializamos el Superbloque al inicio (offset 0)
    if (leidos < (ssize_t)sizeof(DBHeader)) {
        memset(&header, 0, sizeof(DBHeader));
        header.magic = MAGIC_NUMBER;
        header.version = DB_VERSION;
        header.ultimo_id = 0;
        header.total_activos = 0;
        header.total_borrados = 0;
        header.ultima_modif = (uint64_t)time(NULL);

        pwrite(db_fd, &header, sizeof(DBHeader), 0);
        fdatasync(db_fd); // Forzar guardado físico en el disco de Linux
        printf("[MOTOR DB] Archivo '%s' creado e inicializado correctamente.\n", DB_FILE);
    } else if (header.magic != MAGIC_NUMBER) {
        fprintf(stderr, "[ERROR] El archivo '%s' está corrupto o formato no válido.\n", DB_FILE);
        exit(EXIT_FAILURE);
    } else {
        printf("[MOTOR DB] Motor cargado con éxito: %u registros activos | Último ID: %u\n",
               header.total_activos, header.ultimo_id);
    }
}


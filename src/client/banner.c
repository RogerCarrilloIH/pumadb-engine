#include "banner.h"
#include <stdio.h>

/* Códigos de escape ANSI para colores en terminal */
#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_DIM     "\033[2m"
#define ANSI_CYAN    "\033[1;36m"
#define ANSI_YELLOW  "\033[1;33m"
#define ANSI_GREEN   "\033[1;32m"
#define ANSI_BLUE    "\033[1;34m"
#define ANSI_GRAY    "\033[90m"
#define ANSI_MAGENTA "\033[1;35m"

void print_client_banner(const char *server_ip, int port) {
    printf("\n");
    printf(ANSI_GRAY "┌──────────────────────────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET "  " ANSI_DIM "PumaDB-Engie        " ANSI_RESET "                                 " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET "  " ANSI_CYAN "____  __  __ __  __    _      ____  ____ " ANSI_RESET "   " ANSI_YELLOW "/\\_____/\\ " ANSI_RESET "  " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET " " ANSI_CYAN "|  _ \\| / / /|  \\/  |  / \\    |  _ \\| __ )" ANSI_RESET "  " ANSI_YELLOW "/  o   o  \\" ANSI_RESET "  " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET " " ANSI_CYAN "| |_) | | | || |\\/| | / _ \\   | | | |  _ \\" ANSI_RESET " " ANSI_YELLOW "(  == ^ ==  )" ANSI_RESET " " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET " " ANSI_CYAN "|  __/| |_| || |  | |/ ___ \\  | |_| | |_) )" ANSI_RESET "  " ANSI_YELLOW ")       ( " ANSI_RESET "  " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET " " ANSI_CYAN "|_|    \\___/ |_|  |_/_/   \\_\\ |____/|____/" ANSI_RESET "  " ANSI_YELLOW "( ( ) ( ) )" ANSI_RESET "  " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET "  " ANSI_MAGENTA "MOTOR v1.0" ANSI_RESET " - " ANSI_DIM "DBMS en C para Linux" ANSI_RESET "                       " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "└──────────────────────────────────────────────────────────┘\n" ANSI_RESET);
    printf("\n");
    printf(" Cliente PumaDB conectado por sockets TCP/IP.\n");
    printf(" Comandos SQL soportados: " ANSI_BOLD "SELECT" ANSI_RESET ", " ANSI_BOLD "INSERT" ANSI_RESET ", " ANSI_BOLD "UPDATE" ANSI_RESET ", " ANSI_BOLD "DELETE" ANSI_RESET ".\n\n");
    printf(" " ANSI_BLUE "●" ANSI_RESET " Conectado al Servidor: " ANSI_BOLD "%s:%d" ANSI_RESET "\n", server_ip, port);
    printf(" " ANSI_BLUE "●" ANSI_RESET " Tabla activa: " ANSI_BOLD "Alumno (alumno.txt)" ANSI_RESET "\n");
    printf(" " ANSI_GREEN "●" ANSI_RESET " Estado de la sesion: " ANSI_GREEN "Listo" ANSI_RESET "\n\n");
    printf(ANSI_GRAY " Ingrese consulta SQL o " ANSI_BOLD "SALIR" ANSI_RESET ANSI_GRAY " para desconectar." ANSI_RESET "\n");
    printf(ANSI_GRAY " Presione Ctrl+C para salir.\n\n" ANSI_RESET);
}

void print_server_banner(int port) {
    printf("\n");
    printf(ANSI_GRAY "┌──────────────────────────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET "  " ANSI_DIM "PumaDB-Engie        " ANSI_RESET "                                    " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET "  " ANSI_CYAN "____  __  __ __  __    _      ____  ____ " ANSI_RESET "   " ANSI_YELLOW "/\\_____/\\ " ANSI_RESET "  " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET " " ANSI_CYAN "|  _ \\| / / /|  \\/  |  / \\    |  _ \\| __ )" ANSI_RESET "  " ANSI_YELLOW "/  o   o  \\" ANSI_RESET "  " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET " " ANSI_CYAN "| |_) | | | || |\\/| | / _ \\   | | | |  _ \\" ANSI_RESET " " ANSI_YELLOW "(  == ^ ==  )" ANSI_RESET " " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET " " ANSI_CYAN "|  __/| |_| || |  | |/ ___ \\  | |_| | |_) )" ANSI_RESET "  " ANSI_YELLOW ")       ( " ANSI_RESET "  " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET " " ANSI_CYAN "|_|    \\___/ |_|  |_/_/   \\_\\ |____/|____/" ANSI_RESET "  " ANSI_YELLOW "( ( ) ( ) )" ANSI_RESET "  " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "│" ANSI_RESET "  " ANSI_MAGENTA "SERVIDOR DAEMON" ANSI_RESET " - " ANSI_DIM "Concurrente con fork()" ANSI_RESET "                " ANSI_GRAY "│\n" ANSI_RESET);
    printf(ANSI_GRAY "└──────────────────────────────────────────────────────────┘\n" ANSI_RESET);
    printf("\n");
    printf(" " ANSI_GREEN "●" ANSI_RESET " Servidor iniciado en puerto: " ANSI_BOLD "%d" ANSI_RESET "\n", port);
    printf(" " ANSI_BLUE "●" ANSI_RESET " Archivo de datos: " ANSI_BOLD "data/alumno.txt" ANSI_RESET "\n");
    printf(" " ANSI_BLUE "●" ANSI_RESET " Modelo: " ANSI_BOLD "Multiproceso (fork + SIGCHLD)" ANSI_RESET "\n\n");
    printf(ANSI_GRAY " Esperando conexiones de clientes... (Ctrl+C para parar)\n\n" ANSI_RESET);
}

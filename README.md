<pre>
<span style="color: #71767b;">┌──────────────────────────────────────────────────────────┐</span>
<span style="color: #71767b;">│</span>  <span style="color: #8b949e;">PumaDB-Engie</span>                                            <span style="color: #71767b;">│</span>
<span style="color: #71767b;">│</span>  <span style="color: #00e5ff; font-weight: bold;">____  __  __ __  __    _      ____  ____ </span>   <span style="color: #ffd600; font-weight: bold;">/\_____/\ </span>  <span style="color: #71767b;">│</span>
<span style="color: #71767b;">│</span> <span style="color: #00e5ff; font-weight: bold;">|  _ \| / / /|  \/  |  / \    |  _ \| __ )</span>  <span style="color: #ffd600; font-weight: bold;">/  o   o  \</span>  <span style="color: #71767b;">│</span>
<span style="color: #71767b;">│</span> <span style="color: #00e5ff; font-weight: bold;">| |_) | | | || |\/| | / _ \   | | | |  _ \</span> <span style="color: #ffd600; font-weight: bold;">(  == ^ ==  )</span> <span style="color: #71767b;">│</span>
<span style="color: #71767b;">│</span> <span style="color: #00e5ff; font-weight: bold;">|  __/| |_| || |  | |/ ___ \  | |_| | |_) )</span>  <span style="color: #ffd600; font-weight: bold;">)       ( </span>  <span style="color: #71767b;">│</span>
<span style="color: #71767b;">│</span> <span style="color: #00e5ff; font-weight: bold;">|_|    \___/ |_|  |_/_/   \_\ |____/|____/</span>  <span style="color: #ffd600; font-weight: bold;">( ( ) ( ) )</span>  <span style="color: #71767b;">│</span>
<span style="color: #71767b;">│</span>  <span style="color: #d2a8ff; font-weight: bold;">MOTOR v1.0</span> - <span style="color: #8b949e;">DBMS en C para Linux</span>                       <span style="color: #71767b;">│</span>
<span style="color: #71767b;">└──────────────────────────────────────────────────────────┘</span>
</pre>

# PumaDB Engine (DBMS Cliente-Servidor en C)

Sistema Manejador de Bases de Datos (DBMS) relacional bajo arquitectura cliente-servidor para sistemas Linux, implementado en lenguaje C puro con sockets POSIX TCP/IP, concurrencia multiproceso (`fork`) y persistencia sobre archivos planos.

Desarrollado para la asignatura **Arquitecturas Cliente-Servidor (Facultad de Ingeniería, UNAM)**.

---

## Interfaz Web y Demostración Interactiva

Se cuenta con una página web oficial en [pumadb-engie.ddns.net](https://pumadb-engie.ddns.net/), diseñada con un estilo moderno inspirado en herramientas de desarrollo avanzadas:

- **Emulador Interactivo en el Navegador**: Prueba sentencias SQL en tiempo real (`SELECT`, `INSERT`, `UPDATE`, `DELETE`, `SALIR`, `AYUDA`) directamente desde la interfaz, experimentando la misma salida ASCII y diseño visual que el cliente en C.
- **Acceso y Comodidad**: Utiliza los botones preconfigurados para ejecutar consultas con un solo clic y copia los resultados instantáneamente al portapapeles.
- **Galería y Demostración**: Explora el visor modal con capturas reales de la ejecución del cliente y servidor en entornos Linux.

---

## Intrucciones para probar el proyecto

### 1. Compilar el proyecto

Utilice el comando `make` para compilar de forma limpia tanto el servidor como el cliente:

```bash
make clean && make
```

### 2. Ejecutar el Servidor (en una terminal)

Inicie el proceso servidor indicando el puerto de escucha (ejemplo: `8080`):

```bash
./bin/servidor 8080
```

El servidor desplegará el banner inicial y quedará en espera de conexiones entrantes concurrentes mediante `fork()`.

### 3. Ejecutar el Cliente (en otra terminal)

Abra otra ventana de terminal y ejecute el cliente indicando la dirección IP del servidor y el puerto:

```bash
./bin/cliente 127.0.0.1 8080
```

Se desplegará el banner interactivo en español estilo Copilot CLI con el logotipo de PumaDB a la izquierda y el puma a la derecha en arte ASCII calibrado a 60 columnas, seguido del prompt de comandos `>`.

### 4. Comandos de prueba en la consola interactiva

Copie y ejecute la siguiente secuencia de sentencias SQL en la consola del cliente:

```sql
> SELECT * FROM Alumno
> INSERT INTO Alumno VALUES (Jose,Sanchez,9,Telecom)
> UPDATE Alumno SET Apellido=Gomez WHERE Id=2
> DELETE FROM Alumno WHERE Id=2
> SELECT * FROM Alumno
> SALIR
```

---

## Glosario y Documentación Técnica Especializada

Para consultar el análisis detallado, la fundamentación teórica y los diagramas del sistema, navegue a través de la carpeta [`docs/`](docs/):

| Índice / Término | Descripción Técnica | Documento Completo |
| :--- | :--- | :---: |
| **Arquitectura del Sistema** | Diagramas de red TCP, flujo de concurrencia con `fork()`, recolector de zombies con `SIGCHLD` y persistencia atómica. | [-> Ver Arquitectura](docs/arquitectura.md) |
| **Control de Concurrencia y Bloqueos** | Exclusión mutua sobre el archivo `alumno.txt` con `flock(LOCK_SH)` y `flock(LOCK_EX)`. | [-> Ver Bloqueos](docs/arquitectura.md#3-persistencia-y-control-de-concurrencia-flock) |
| **Trazabilidad de Requerimientos** | Matriz de cumplimiento frente a las especificaciones del documento oficial del examen de la UNAM. | [-> Ver Requerimientos](docs/analisis-requerimientos.md) |
| **Especificación de Módulos y Archivos** | Descripción archivo por archivo, prototipos de funciones, parámetros, valores de retorno y tratamiento de errores. | [-> Ver Módulos](docs/modulos-archivos.md) |
| **Glosario de Conceptos de Sistemas** | Definición formal de conceptos clave (Sockets POSIX, Procesos Zombies, Atomicidad, REPL, Códigos ANSI). | [-> Ver Glosario](docs/glosario.md) |

---

## Estructura del Proyecto

El código fuente está estrictamente organizado de forma modular para garantizar claridad y facilidad de evaluación:

```text
pumadb-engine/
├── Makefile                 # Automatización de compilación con banderas estrictas (-std=c99 -Wall)
├── README.md                # Guía de uso, prueba rápida y glosario de navegación
├── data/                    # Directorio de persistencia de la base de datos
│   └── alumno.txt           # Archivo que simula la tabla Alumno (campos separados por comas)
├── bin/                     # Directorio de binarios ejecutables compilados
│   ├── servidor             # Servidor DBMS concurrente
│   └── cliente              # Cliente interactivo de terminal
├── docs/                    # Documentación técnica especializada
│   ├── arquitectura.md      # Diagramas de flujo, sockets TCP y modelo fork()
│   ├── analisis-requerimientos.md # Matriz de trazabilidad con el PDF del examen
│   ├── modulos-archivos.md  # Especificación técnica exhaustiva de funciones y archivos
│   └── glosario.md          # Glosario conceptual de sistemas operativos y redes
└── src/
    ├── common/              # Definiciones y utilidades compartidas
    │   ├── protocol.h       # Constantes del protocolo, límites de búfer y respuestas
    │   ├── utils.h          # Prototipos de funciones auxiliares y sockets
    │   └── utils.c          # Implementación de saneamiento de cadenas y transmisión TCP
    ├── server/              # Módulos del servidor DBMS
    │   ├── servidor.c       # Main, bucle TCP, concurrencia fork() y recolector SIGCHLD
    │   ├── db_engine.h      # Interfaz de funciones CRUD sobre alumno.txt
    │   ├── db_engine.c      # Persistencia con flock(), autoincremento y reemplazo atómico
    │   ├── parser.h         # Interfaz del intérprete de comandos SQL
    │   └── parser.c         # Análisis sintáctico y extracción de parámetros SQL
    └── client/              # Módulos del cliente interactivo
        ├── banner.h         # Prototipos del banner gráfico de terminal
        ├── banner.c         # Banner estilo Copilot CLI con arte ASCII y colores ANSI
        └── cliente.c        # Main del cliente, gestión de sockets y bucle REPL
```

---

## Resumen de Comandos SQL Soportados

| Comando | Sintaxis de Ejemplo | Acción en la Base de Datos |
| :--- | :--- | :--- |
| **SELECT** | `SELECT * FROM Alumno` | Retorna todos los registros de la tabla bajo bloqueo compartido `LOCK_SH`. |
| **INSERT** | `INSERT INTO Alumno VALUES (Jose,Sanchez,9,Telecom)` | Inserta un registro al final con `Id` autoincremental (`max(Id) + 1`). Cadenas sin comillas. |
| **UPDATE** | `UPDATE Alumno SET Apellido=Gomez WHERE Id=2` | Actualiza la columna indicada. **La columna Id no puede ser modificada**. |
| **DELETE** | `DELETE FROM Alumno WHERE Id=2` | Remueve de forma atómica el registro correspondiente de `alumno.txt`. |
| **SALIR** | `SALIR` | Desconecta ordenadamente la sesión del cliente y libera los recursos del socket. |

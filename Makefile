CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c99 -D_DEFAULT_SOURCE -D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE=200809L -Isrc/common -Isrc/server -Isrc/client -O2

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

COMMON_SRCS = $(SRC_DIR)/common/utils.c $(SRC_DIR)/client/banner.c
SERVER_SRCS = $(SRC_DIR)/server/servidor.c $(SRC_DIR)/server/db_engine.c $(SRC_DIR)/server/parser.c $(COMMON_SRCS)
CLIENT_SRCS = $(SRC_DIR)/client/cliente.c $(COMMON_SRCS)

SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))
CLIENT_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))

SERVER_BIN = $(BIN_DIR)/servidor
CLIENT_BIN = $(BIN_DIR)/cliente

.PHONY: all server client clean directories run-server run-client

all: directories server client

directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)/common $(OBJ_DIR)/server $(OBJ_DIR)/client
	@mkdir -p data

server: directories $(SERVER_BIN)

client: directories $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(CLIENT_BIN): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run-server: server
	./$(SERVER_BIN) 8080

run-client: client
	./$(CLIENT_BIN) 127.0.0.1 8080

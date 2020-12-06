BUILD_DIR=build
BIN_DIR=$(BUILD_DIR)/bin
SRC_DIR=./src

CC=gcc

CLIENT_O=$(BUILD_DIR)/client.o

CLIENT_C=$(SRC_DIR)/main.c

clear:
	rm -R $(BUILD_DIR) -f
	rm -R $(BIN_DIR) -f

mkd: clear
	mkdir $(BUILD_DIR)
	mkdir $(BIN_DIR)

compile: mkd
	$(CC) $(CLIENT_C) -c -o $(CLIENT_O)

build: compile
	$(CC) -o $(BIN_DIR)/client.bin $(CLIENT_O) -lpthread
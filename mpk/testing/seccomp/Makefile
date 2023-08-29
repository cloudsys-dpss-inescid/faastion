.PHONY: all run clean

CC := gcc
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
CFLAGS := -g -Wall
MODE := --mode-mpk

all: $(BUILD_DIR) $(BIN_DIR)/test_seccomp $(BIN_DIR)/test_application

$(BUILD_DIR):
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Executables
$(BIN_DIR)/test_seccomp: $(OBJ_DIR)/test_seccomp.o $(OBJ_DIR)/seccomp.o
	$(CC) $(CFLAGS) -o $@ $^ -pthread

$(BIN_DIR)/test_application: test_application.c
	$(CC) $(CFLAGS) -c $< -o $@

# Object Files
$(OBJ_DIR)/test_seccomp.o: test_seccomp.c
	$(CC) $(CFLAGS) -Iutils -c $< -o $@

$(OBJ_DIR)/seccomp.o: utils/seccomp.c
	$(CC) $(CFLAGS) -DSECC_DBG -c $< -o $@

run: all
	$(BIN_DIR)/test_seccomp $(MODE) $(BIN_DIR)/test_application

clean:
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/* seccomp.out

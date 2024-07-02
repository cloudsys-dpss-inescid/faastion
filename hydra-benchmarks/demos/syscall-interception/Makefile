.PHONY: all clean

CC := gcc
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
CFLAGS := -g -Wall

all: $(BUILD_DIR) $(BIN_DIR)/test_seccomp

$(BUILD_DIR):
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Executables
$(BIN_DIR)/test_seccomp: $(OBJ_DIR)/test_seccomp.o $(OBJ_DIR)/seccomp.o
	$(CC) $(CFLAGS) -o $@ $^ -pthread

# Object Files
$(OBJ_DIR)/test_seccomp.o: seccomp/test_seccomp.c
	$(CC) $(CFLAGS) -Iutils -c $< -o $@

$(OBJ_DIR)/seccomp.o: utils/seccomp.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/*
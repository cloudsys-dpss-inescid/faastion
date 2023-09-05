CC := gcc
CFLAGS := -g -Wall
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

#PATH_TO_LD = ../../ld-preload
#LD_BIN_DIR = $(PATH_TO_LD)/build/bin
#
#PATH_TO_ERIM = ../../erim
#ERIM_SRC_DIR = $(PATH_TO_ERIM)/src
#ERIM_BIN_DIR = $(PATH_TO_ERIM)/bin
#LIBRARIES = $(ERIM_BIN_DIR)/common/libswscommon.a $(ERIM_BIN_DIR)/erim/liberim.a
#LDLIBS = -lm $(LIBRARIES)

.PHONY: all run gdb clean

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
	$(BIN_DIR)/test_seccomp $(BIN_DIR)/test_application

gdb: all
	gdb -ex run --args $(BIN_DIR)/test_seccomp $(BIN_DIR)/test_application

clean:
	@echo Cleaning...
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/* seccomp.out

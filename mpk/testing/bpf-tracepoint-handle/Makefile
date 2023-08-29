.PHONY: all clean

CC := clang
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
SKEL_DIR := $(BUILD_DIR)/skel

all: $(BUILD_DIR) $(BIN_DIR)/hello

$(BUILD_DIR):
	mkdir -p $(OBJ_DIR) $(BIN_DIR) $(SKEL_DIR)

# Executables
$(BIN_DIR)/hello: hello/hello.c $(SKEL_DIR)/hello.skel.h
	$(CC) -I$(SKEL_DIR) -o $@ $< -lbpf -lelf

# Skeletons
$(SKEL_DIR)/hello.skel.h: $(OBJ_DIR)/hello.bpf.o
	bpftool gen skeleton $< name hello > $@

# Object Files
$(OBJ_DIR)/hello.bpf.o: hello/hello.bpf.c
	$(CC) -g -O3 -target bpf -D__TARGET_ARCH_x86_64 -c $< -o $@

clean:
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/* $(SKEL_DIR)/*

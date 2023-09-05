CC = gcc
CFLAGS = -Wall -g -fno-inline -fPIC
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

PATH_TO_ERIM = ../../erim
ERIM_SRC_DIR = $(PATH_TO_ERIM)/src
ERIM_BIN_DIR = $(PATH_TO_ERIM)/bin
EFLAGS = -I$(PATH_TO_ERIM)/src/erim -I$(PATH_TO_ERIM)/src/common -DERIM_DBG -DERIM_SWAP_STACKS
LIBRARIES=$(PATH_TO_ERIM)/bin/common/libswscommon.a $(PATH_TO_ERIM)/bin/erim/liberim.a
LDLIBS=-lm $(LIBRARIES)

.PHONY: all erim clean

all: erim $(BUILD_DIR) $(BIN_DIR)/libpreload.so

$(BUILD_DIR):
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Executables
$(BIN_DIR)/libpreload.so: erim utils/appmap.o utils/threadmap.o helpers/helpers.o preload.o
	$(CC) $(CFLAGS) $(EFLAGS) -shared -o $@ $^ $(LDLIBS)

# Object Files
$(OBJ_DIR)/utils/appmap.o: utils/appmap.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/utils/threadmap.o: utils/threadmap.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/helpers/helpers.o: helpers/helpers.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/preload.o: preload.c
	$(CC) $(CFLAGS) -o $@ -c $<

erim:
	$(MAKE) -C $(ERIM_SRC_DIR) clean && $(MAKE) -C $(ERIM_SRC_DIR)
	
clean:
	@echo Cleaning...
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/*

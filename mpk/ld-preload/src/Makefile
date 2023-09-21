CC = gcc
CFLAGS = -Wall -g -fno-inline -fPIC
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

PATH_TO_ERIM = ../../../erim
INCLUDES := -I$(PATH_TO_ERIM)/src/erim -I$(PATH_TO_ERIM)/src/common 
LIBRARIES := $(PATH_TO_ERIM)/bin/common/libswscommon.a $(PATH_TO_ERIM)/bin/erim/liberim.a

.PHONY: all clean

all: $(BUILD_DIR) $(BIN_DIR)/libpreload.so

$(BUILD_DIR):
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Executables
$(BIN_DIR)/libpreload.so: $(OBJ_DIR)/appmap.o $(OBJ_DIR)/threadmap.o $(OBJ_DIR)/helpers.o $(OBJ_DIR)/preload.o
	$(CC) $(CFLAGS) $ -shared -o $@ $^ -lm $(LIBRARIES) -DERIM_DBG -DERIM_SWAP_STACKS

# Object Files
$(OBJ_DIR)/appmap.o: utils/appmap.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/threadmap.o: utils/threadmap.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/helpers.o: helpers/helpers.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/preload.o: preload.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $< -DSEC_DBG

clean:
	@echo Cleaning...
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/*

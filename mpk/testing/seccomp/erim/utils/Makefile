
PATH_TO_ERIM = ../../erim
ERIM_SRC_DIR = $(PATH_TO_ERIM)/src
ERIM_BIN_DIR = $(PATH_TO_ERIM)/bin
LIBRARIES = $(ERIM_BIN_DIR)/common/libswscommon.a $(ERIM_BIN_DIR)/erim/liberim.a
LDLIBS = -lm $(LIBRARIES)

CC := gcc
EFLAGS := -O0 -pthread -fno-inline -I$(ERIM_SRC_DIR)/erim -I$(ERIM_SRC_DIR)/common -DERIM_DBG -DERIM_SWAP_STACKS
CFLAGS := -g -Wall $(EFLAGS)

LIBRARIES = $(ERIM_BIN_DIR)/common/libswscommon.a $(ERIM_BIN_DIR)/erim/liberim.a
LDLIBS = -lm $(LIBRARIES)

.PHONY: all run clean

all: 
	$(CC) $(CFLAGS) -DSECC_DBG seccomp.c -o test $(LDLIBS)

run:
	./test

clean:
	@echo Cleaning...
	rm -f test

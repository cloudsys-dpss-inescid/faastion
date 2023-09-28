PATH_TO_ERIM=../../../../erim

CC = gcc
CFLAGS = -Wall -g -fno-inline -I$(PATH_TO_ERIM)/src/erim -I$(PATH_TO_ERIM)/src/common -DERIM_DBG -DERIM_SWAP_STACKS

LIBRARIES =$(PATH_TO_ERIM)/bin/common/libswscommon.a $(PATH_TO_ERIM)/bin/erim/liberim.a
LDLIBS = -lm $(LIBRARIES)

TARGET = test

.PHONY: build erim run clean

build: erim
	$(CC) $(CFLAGS) -c test_application.c -o test_application.o
	$(CC) -o $(TARGET) test_application.o $(LDLIBS)

erim:
	$(MAKE) clean -C $(PATH_TO_ERIM)/src && $(MAKE) -C $(PATH_TO_ERIM)/src

run:
	./$(TARGET)

clean:
	rm -f $(TARGET)

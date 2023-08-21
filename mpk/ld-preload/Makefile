PATH_TO_ERIM=../erim

CC = gcc
CFLAGS = -Wall -g -I.. -fno-inline -fPIC -I$(PATH_TO_ERIM)/src/erim -I$(PATH_TO_ERIM)/src/common -DERIM_DBG -DERIM_SWAP_STACKS

LIBRARIES=$(PATH_TO_ERIM)/bin/common/libswscommon.a $(PATH_TO_ERIM)/bin/erim/liberim.a
LDLIBS=-lm $(LIBRARIES)

.PHONY: all clean

all: libpreload.so

libpreload.so: utils/appmap.o utils/threadmap.o helpers/helpers.o preload.o
	$(CC) $(CFLAGS) -shared -o libpreload.so utils/appmap.o utils/threadmap.o helpers/helpers.o preload.o $(LDLIBS)

utils/appmap.o: utils/appmap.c utils/appmap.h
	$(CC) $(CFLAGS) -o utils/appmap.o -c utils/appmap.c

utils/threadmap.o: utils/threadmap.c utils/threadmap.h
	$(CC) $(CFLAGS) -o utils/threadmap.o -c utils/threadmap.c

helpers/helpers.o: helpers/helpers.c helpers/helpers.h
	$(CC) $(CFLAGS) -o helpers/helpers.o -c helpers/helpers.c

preload.o: preload.c utils/appmap.h
	$(CC) $(CFLAGS) -o preload.o -c preload.c

clean:
	@echo Cleaning...
	rm -f helpers/*.o utils/*.o *.o libpreload.so

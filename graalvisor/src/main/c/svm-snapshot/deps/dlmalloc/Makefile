dlflags = -DHAVE_MORECORE=0 -DUSE_DL_PREFIX -DMSPACES -DMALLOC_INSPECT_ALL=1 -DUSE_DLMALLOC_MALLOC_TRIM -DFOOTERS=1 # for mspace_inspect_all

all: malloc.c malloc.h
	gcc -O0 -g3 -Wall $(dlflags) -Wno-maybe-uninitialized -fPIC -c malloc.c
	gcc -O0 -g3 -Wall $(dlflags) -Wno-maybe-uninitialized -fPIC -c cr_malloc.c
	gcc -O0 -g3 -Wall -fPIC -shared $(dlflags) -Wno-maybe-uninitialized -o hydralloc.so malloc.o cr_malloc.o ../printf/printf.o ../../cr_logger.o -ldl

test: all cr_malloc.c cr_malloc.h main.c
	gcc -O0 -g3 -Wall $(dlflags) -Wno-maybe-uninitialized  -c main.c
	gcc -O0 -g3 -Wall -o main main.o cr_malloc.o malloc.o ../printf/printf.o ../../cr_logger.o

clean:
	rm -f *.o main hydralloc.so

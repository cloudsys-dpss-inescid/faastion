.PHONY: all clean

CC			= gcc
CFLAGS		= -g -Wall -fPIC
OPTIONS		= -DHAVE_MORECORE=0 -DUSE_DL_PREFIX -DONLY_MSPACES -DMALLOC_INSPECT_ALL=1 # for mspace_inspect_all
LIB_DIR		= $(ARGO_HOME)/graalvisor/build/libs
SRC_DIR		= $(ARGO_HOME)/graalvisor/src/main/c
PKRU_DIR	= $(SRC_DIR)/pkru-sandbox
MALLOC_DIR	= $(SRC_DIR)/dlmalloc
INCLUDE 	= -I$(LIBC_HOME)/include -I$(DEF_JAVA_HOME)/include -I$(DEF_JAVA_HOME)/include/linux -I$(PKRU_DIR) -I$(MALLOC_DIR) -I$(ARGO_HOME)/graalvisor/build/generated/sources/headers/java/main

all:
	$(CC) $(CFLAGS) $(INCLUDE) $(OPTIONS) -Wno-maybe-uninitialized -c -o $(LIB_DIR)/malloc.o $(MALLOC_DIR)/malloc.c
	$(CC) $(CFLAGS) $(INCLUDE) $(OPTIONS) -Wno-maybe-uninitialized -c -o $(LIB_DIR)/cr_malloc.o $(MALLOC_DIR)/cr_malloc.c
#
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/loader.o $(SRC_DIR)/loader/loader.c
	$(CC) -g -shared -o $(LIB_DIR)/libloader.so $(LIB_DIR)/loader.o $(LIB_DIR)/cr_malloc.o $(LIB_DIR)/malloc.o 
#
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/domain_manager.o $(PKRU_DIR)/domain_manager.c
	$(CC) $(CFLAGS) $(INCLUDE) -DREMOVE_NNS_LIMIT -c -o $(LIB_DIR)/memory_map.o $(PKRU_DIR)/memory_map.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/main.o $(PKRU_DIR)/main.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/hash_table.o $(PKRU_DIR)/hash_table.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/sandbox.o $(PKRU_DIR)/sandbox.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/book_keeper.o $(PKRU_DIR)/book_keeper.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/seccomp.o $(PKRU_DIR)/seccomp.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/worker.o $(PKRU_DIR)/worker.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/jni_monitor.o $(PKRU_DIR)/jni_monitor.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/jvm_monitor.o $(PKRU_DIR)/jvm_monitor.c
#
	$(CC) -g -shared -o $(LIB_DIR)/libpkru.so $(LIB_DIR)/domain_manager.o \
		$(LIB_DIR)/memory_map.o $(LIB_DIR)/main.o $(LIB_DIR)/hash_table.o $(LIB_DIR)/sandbox.o \
		$(LIB_DIR)/book_keeper.o $(LIB_DIR)/seccomp.o $(LIB_DIR)/worker.o $(LIB_DIR)/jni_monitor.o \
		$(LIB_DIR)/jvm_monitor.o $(LIB_DIR)/malloc.o $(LIB_DIR)/cr_malloc.o
#
	$(CC) $(INCLUDE) -c -o $(LIB_DIR)/NativeSandboxInterface.o $(SRC_DIR)/NativeSandboxInterface.c
	ar rcs $(LIB_DIR)/libNativeSandboxInterface.a $(LIB_DIR)/NativeSandboxInterface.o

clean:
	rm -f $(LIB_DIR)/libpkru.so $(LIB_DIR)/*.o


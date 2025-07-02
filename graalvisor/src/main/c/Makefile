.PHONY: all clean gen_wrappers

CC			= gcc
CFLAGS		= -g -Wall -fPIC
OPTIONS		= -DHAVE_MORECORE=0 -DUSE_DL_PREFIX -DONLY_MSPACES -DMALLOC_INSPECT_ALL=1 # for mspace_inspect_all
LIB_DIR		= $(ARGO_HOME)/graalvisor/build/libs
SRC_DIR		= $(ARGO_HOME)/graalvisor/src/main/c
PKRU_DIR	= $(SRC_DIR)/pkru-sandbox
MALLOC_DIR	= $(SRC_DIR)/dlmalloc
LOADER_DIR 	= $(SRC_DIR)/loader
WRAPPER_DIR = $(SRC_DIR)/jni
NATIVE_INTERFACE_DIR = $(SRC_DIR)/native-interface
INCLUDE 	= -I$(LIBC_HOME)/include -I$(DEF_JAVA_HOME)/include -I$(DEF_JAVA_HOME)/include/linux \
	-I$(PKRU_DIR) -I$(MALLOC_DIR) -I$(WRAPPER_DIR) -I$(ARGO_HOME)/graalvisor/build/generated/sources/headers/java/main

all: gen_wrappers
	$(CC) $(CFLAGS) $(INCLUDE) -DLOADER_LIB="\"$(LIB_DIR)/libloader.so\"" -DREMOVE_NNS_LIMIT -c -o $(LIB_DIR)/jni_handle.o $(NATIVE_INTERFACE_DIR)/jni_handle.c

	$(CC) $(CFLAGS) $(INCLUDE) $(OPTIONS) -Wno-maybe-uninitialized -c -o $(LIB_DIR)/malloc.o $(MALLOC_DIR)/malloc.c
	$(CC) $(CFLAGS) $(INCLUDE) $(OPTIONS) -Wno-maybe-uninitialized -c -o $(LIB_DIR)/cr_malloc.o $(MALLOC_DIR)/cr_malloc.c
	$(CC) $(CFLAGS) $(INCLUDE) $(OPTIONS) -Wno-maybe-uninitialized -c -o $(LIB_DIR)/log.o $(MALLOC_DIR)/log.c
	$(CC) $(CFLAGS) $(INCLUDE) $(OPTIONS) -Wno-maybe-uninitialized -c -o $(LIB_DIR)/malloc_sync.o $(MALLOC_DIR)/malloc_sync.c
	$(CC) -g -shared -o $(LIB_DIR)/libmem.so $(LIB_DIR)/cr_malloc.o $(LIB_DIR)/malloc.o $(LIB_DIR)/log.o $(LIB_DIR)/malloc_sync.o
#
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/loader.o $(LOADER_DIR)/loader.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/libc_wrappers.o $(LOADER_DIR)/libc_wrappers.c
	$(CC) -g -shared -o $(LIB_DIR)/libloader.so $(LIB_DIR)/loader.o $(LIB_DIR)/libc_wrappers.o $(LIB_DIR)/cr_malloc.o $(LIB_DIR)/malloc.o $(LIB_DIR)/log.o $(LIB_DIR)/malloc_sync.o
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
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(LIB_DIR)/jni_wrapper.o $(WRAPPER_DIR)/jni_wrapper.c
#
	$(CC) -g -shared -o $(LIB_DIR)/libpkru.so $(LIB_DIR)/domain_manager.o \
		$(LIB_DIR)/memory_map.o $(LIB_DIR)/main.o $(LIB_DIR)/hash_table.o $(LIB_DIR)/sandbox.o \
		$(LIB_DIR)/book_keeper.o $(LIB_DIR)/seccomp.o $(LIB_DIR)/worker.o $(LIB_DIR)/jni_monitor.o \
		$(LIB_DIR)/jvm_monitor.o $(LIB_DIR)/jni_wrapper.o
#
	$(CC) $(INCLUDE) -c -o $(LIB_DIR)/NativeSandboxInterface.o $(SRC_DIR)/NativeSandboxInterface.c
	ar rcs $(LIB_DIR)/libNativeSandboxInterface.a $(LIB_DIR)/NativeSandboxInterface.o

gen_wrappers: generate_libc_wrappers.sh generate_libc_wrappers.py
	./generate_libc_wrappers.sh

clean:
	rm -f $(LIB_DIR)/libpkru.so $(LIB_DIR)/*.o


.PHONY: all clean gen_wrappers native_sandbox pku_sandbox create_dirs

CC			= gcc
CFLAGS		= -g -Wall -fPIC
OPTIONS		= -DHAVE_MORECORE=0 -DUSE_DL_PREFIX -DMSPACES -DMALLOC_INSPECT_ALL=1 -DUSE_DLMALLOC_MALLOC_TRIM -DFOOTERS=1 # for mspace_inspect_all
OBJ_DIR		= $(ARGO_HOME)/core/build/obj
LIB_DIR		= $(ARGO_HOME)/core/shared
SRC_DIR		= $(ARGO_HOME)/core/src/main/c
PKRU_DIR	= $(SRC_DIR)/pkru-sandbox
MALLOC_DIR	= $(SRC_DIR)/dlmalloc
LOADER_DIR 	= $(SRC_DIR)/loader
WRAPPER_DIR = $(SRC_DIR)/jni
NATIVE_INTERFACE_DIR = $(SRC_DIR)/native-interface
NET_DIR 	= $(SRC_DIR)/network-isolation/src
SNAP_DIR	= $(SRC_DIR)/svm-snapshot
INCLUDE 	= -I$(LIBC_HOME)/include -I$(DEF_JAVA_HOME)/include -I$(DEF_JAVA_HOME)/include/linux \
	-I$(PKRU_DIR) -I$(MALLOC_DIR) -I$(WRAPPER_DIR) -I$(ARGO_HOME)/core/build/generated/sources/headers/java/main

pku_sandbox: create_dirs gen_wrappers
	$(CC) $(CFLAGS) $(INCLUDE) -DLOADER_LIB="\"$(LIB_DIR)/libloader.so\"" -DREMOVE_NNS_LIMIT -c -o $(OBJ_DIR)/jni_handle.o $(NATIVE_INTERFACE_DIR)/jni_handle.c

	$(CC) $(CFLAGS) $(INCLUDE) $(OPTIONS) -Wno-maybe-uninitialized -c -o $(OBJ_DIR)/malloc.o $(MALLOC_DIR)/malloc.c
	$(CC) $(CFLAGS) $(INCLUDE) $(OPTIONS) -Wno-maybe-uninitialized -c -o $(OBJ_DIR)/cr_malloc.o $(MALLOC_DIR)/cr_malloc.c
	$(CC) $(CFLAGS) $(INCLUDE) $(OPTIONS) -Wno-maybe-uninitialized -c -o $(OBJ_DIR)/log.o $(MALLOC_DIR)/log.c
	$(CC) $(CFLAGS) $(INCLUDE) $(OPTIONS) -Wno-maybe-uninitialized -c -o $(OBJ_DIR)/malloc_sync.o $(MALLOC_DIR)/malloc_sync.c
	$(CC) -g -shared -o $(LIB_DIR)/libmem.so $(OBJ_DIR)/cr_malloc.o $(OBJ_DIR)/malloc.o $(OBJ_DIR)/log.o $(OBJ_DIR)/malloc_sync.o
#
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/loader.o $(LOADER_DIR)/loader.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/libc_wrappers.o $(LOADER_DIR)/libc_wrappers.c
	$(CC) -g -shared -Wl,--no-as-needed -L$(LIB_DIR) -o $(LIB_DIR)/libloader.so $(OBJ_DIR)/libc_wrappers.o $(OBJ_DIR)/loader.o -lmem -lm
#	$(CC) -g -shared -L$(LIB_DIR) -o $(LIB_DIR)/libloader.so $(LIB_DIR)/loader.o -lmem
#
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/domain_manager.o $(PKRU_DIR)/domain_manager.c
	$(CC) $(CFLAGS) $(INCLUDE) -DREMOVE_NNS_LIMIT -c -o $(OBJ_DIR)/memory_map.o $(PKRU_DIR)/memory_map.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/main.o $(PKRU_DIR)/main.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/hash_table.o $(PKRU_DIR)/hash_table.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/sandbox.o $(PKRU_DIR)/sandbox.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/book_keeper.o $(PKRU_DIR)/book_keeper.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/seccomp.o $(PKRU_DIR)/seccomp.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/worker.o $(PKRU_DIR)/worker.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/jni_monitor.o $(PKRU_DIR)/jni_monitor.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/jvm_monitor.o $(PKRU_DIR)/jvm_monitor.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/jni_wrapper.o $(WRAPPER_DIR)/jni_wrapper.c
#
	$(CC) -g -shared -o $(LIB_DIR)/libpkru.so $(OBJ_DIR)/domain_manager.o \
		$(OBJ_DIR)/memory_map.o $(OBJ_DIR)/main.o $(OBJ_DIR)/hash_table.o $(OBJ_DIR)/sandbox.o \
		$(OBJ_DIR)/book_keeper.o $(OBJ_DIR)/seccomp.o $(OBJ_DIR)/worker.o $(OBJ_DIR)/jni_monitor.o \
		$(OBJ_DIR)/jvm_monitor.o $(OBJ_DIR)/jni_wrapper.o

native_sandbox: create_dirs
	$(CC) $(INCLUDE) -I$(NET_DIR) -I$(SNAP_DIR) -c -o $(OBJ_DIR)/NativeSandboxInterface.o $(SRC_DIR)/NativeSandboxInterface.c
	ar rcs $(ARGO_HOME)/core/build/libs/libNativeSandboxInterface.a $(OBJ_DIR)/NativeSandboxInterface.o

all: create_dirs pku_sandbox native_sandbox

create_dirs: $(OBJ_DIR) $(LIB_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

gen_wrappers: generate_libc_wrappers.sh generate_libc_wrappers.py
	./generate_libc_wrappers.sh

clean:
	rm -f $(LIB_DIR)/libpkru.so $(LIB_DIR)/*.o


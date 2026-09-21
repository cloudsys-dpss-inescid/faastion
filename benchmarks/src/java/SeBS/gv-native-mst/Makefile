.PHONY: all clean

SNIPPETS_DIR = build/snippets

SRCS = $(wildcard $(SNIPPETS_DIR)/*.c)
OBJS = $(SRCS:.c=.o)

CC	= gcc
CFLAGS = -g -O0
CFLAGS = -Wall 
CFLAGS += -fPIC -fno-inline
CFLAGS += -I$(DEF_JAVA_HOME)/include -I$(DEF_JAVA_HOME)/include/linux

GRAALVISOR_HOME 	= $(ARGO_HOME)/graalvisor
JNI_WRAPPER_SRC_DIR	= $(GRAALVISOR_HOME)/src/main/c/jni
JNI_HANDLE_SRC_DIR	= $(GRAALVISOR_HOME)/src/main/c/native-interface
PKRU_SRC_DIR 		= $(GRAALVISOR_HOME)/src/main/c/pkru-sandbox
CFLAGS += -I$(PKRU_SRC_DIR) -I$(JNI_WRAPPER_SRC_DIR) -I$(JNI_HANDLE_SRC_DIR)

LIB_CFLAGS = -g -O0
LIB_CFLAGS += -shared
LIB_CFLAGS += -Wall -Wl,--no-undefined

OBJ_DIR = $(GRAALVISOR_HOME)/build/obj
LIB_DIR = $(GRAALVISOR_HOME)/shared

all: $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(CC) $(LIB_CFLAGS) -o $(LIB_DIR)/lib$(BENCHMARK_NAME)-wrapper.so $^ $(OBJ_DIR)/jni_handle.o -L$(LIB_DIR) -lpkru

clean:
	rm -r $(SNIPPETS_DIR)/*.o



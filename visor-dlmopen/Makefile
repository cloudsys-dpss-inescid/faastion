CC = gcc
CFLAGS = -g3 -Wall
CORE_DIR = core
CORE_OBJS = $(CORE_DIR)/pkru_sandbox.o $(CORE_DIR)/memory_map.o $(CORE_DIR)/domain_manager.o

.PHONY: all run clean

all: libapp.so visor

libapp.so: app.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

# Rule to build visor executable, linking core object files
visor: visor.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o visor visor.c $(CORE_OBJS) -pthread

# Rule to compile core .c files into .o object files
$(CORE_DIR)/%.o: $(CORE_DIR)/%.c $(CORE_DIR)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	strace -f -o strace.log setarch -R env ./visor

run-gdb:
	setarch -R gdb -x gdb-commands.txt --args ./visor

clean:
	rm -f visor *.so $(CORE_DIR)/*.o *.log

requirements: create_bin create_snippets build_preload_lib build_javassist_agent

# Make directories
create_bin:
	mkdir -p bin

create_snippets:
	mkdir -p snippets

# Build packages
build_preload_lib:
	make -C $(PRELOAD_HOME)
	cp $(PRELOAD_HOME)/build/bin/libpreload.so $(PATH_TO_BIN)

build_javassist_agent:
	cd $(JAVASSIST_HOME) && mvn -q package

# Compile files
javassist:
	java -javaagent:$(JAVA_AGENT)=$(TOOL):$(CLASS_PATH):$(PATH_TO_BIN) $(ENTRYPOINT)

snippets:
	for file in $$(find "snippets" -type f -name "*.c"); do \
		name=$$(basename "$$file" .c); \
		$(CC) $(SFLAGS) -o "$(PATH_TO_BIN)/lib$$name.so" "$$file" -L$(PATH_TO_BIN) -lpreload; \
	done

# Execution
run:
	export LD_LIBRARY_PATH=$(PATH_TO_BIN)
	LD_PRELOAD=$(PATH_TO_BIN)/libpreload.so java -Djava.library.path=$(PATH_TO_BIN) $(PATH_TO_BIN)/$(ENTRYPOINT)
	
gdb:
	export LD_LIBRARY_PATH=$(PATH_TO_BIN)
	gdb java \
		-ex "set exec-wrapper env 'LD_PRELOAD=$(PATH_TO_BIN)/libpreload.so'" \
		-ex "run -Djava.library.path=$(PATH_TO_BIN) $(PATH_TO_BIN)/$(ENTRYPOINT)"

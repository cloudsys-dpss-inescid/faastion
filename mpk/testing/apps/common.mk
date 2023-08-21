create_snippets:
	mkdir -p snippets

create_bin:
	mkdir -p bin

build_javassist_agent:
	cd $(PATH_TO_JAVASSIST) && mvn -q package

libpreload.so:
	make -C $(PATH_TO_PRELOAD)
	mv $(PATH_TO_PRELOAD)/libpreload.so $(PATH_TO_BIN)
	make clean -C $(PATH_TO_PRELOAD)

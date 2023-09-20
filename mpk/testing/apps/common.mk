create_snippets:
	mkdir -p snippets

create_bin:
	mkdir -p bin

build_javassist_agent:
	cd $(PATH_TO_JAVASSIST) && mvn -q package

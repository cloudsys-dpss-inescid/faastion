PATH_TO_ROOT=.
PATH_TO_BIN=$(PATH_TO_ROOT)/$(OUTPUT)

include flags.mk
include common.mk

CC=gcc
INCLUDE_PATH= -I. -fno-inline

TESTCASES=test_application test_application_integrity
TESTBINARIES=$(addprefix $(PATH_TO_BIN)/, $(TESTCASES)) 

LIBRARIES=$(PATH_TO_ROOT)/common/libswscommon.a $(PATH_TO_ROOT)/erim/liberim.a
LDLIBS= -lm $(LIBRARIES)

.PHONY: all run clean

all: createoutput $(TESTBINARIES) run

$(PATH_TO_BIN)/test_application_integrity: test_application_integrity
	mv $< $(PATH_TO_BIN)
	
$(PATH_TO_BIN)/test_application: test_application
	mv $< $(PATH_TO_BIN)

run: createoutput $(TESTBINARIES)
	for test in $(TESTBINARIES); do\
		./$$test; \
	done

clean:
	rm -f *.o
	rm -f $(PATH_TO_BIN)/*

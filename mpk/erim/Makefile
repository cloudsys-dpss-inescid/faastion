PATH_TO_ERIM=~/erim

all:
	make -C $(PATH_TO_ERIM)/src
	cp $(PATH_TO_ERIM)/src/erim/*.h .
	cp $(PATH_TO_ERIM)/src/common/common.h .
	cp $(PATH_TO_ERIM)/bin/erim/liberim.a .
	cp $(PATH_TO_ERIM)/bin/common/libswscommon.a .
	make clean -C $(PATH_TO_ERIM)/src/erim
	make clean -C $(PATH_TO_ERIM)/src/common
#!/bin/bash

if [ $# -eq 0 ]; then
	echo "usage $0 iter_count"
	exit 1
fi

backup=backup/$(date +%Y.%m.%d.%H.%M.%S)
mkdir -p $backup

for d in 'native' 'seccomp' 'seccomp_allow_rw' 'strace' ; do
	pushd $d > /dev/null
	find -type f -name '*.log' -delete
	./run.sh $@
	popd > /dev/null
	cp --parent $d/*.log $backup &> /dev/null
done


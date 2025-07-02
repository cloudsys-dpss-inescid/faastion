#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

HDRS=()
FILE=$DIR/libc_wrappers.c

rm -f $FILE

echo "#define _GNU_SOURCE" >> $FILE

function get_header_files {
    for file in $(ls $LIBC_HOME/$1 | grep -e '\.h'\$)
    do
        if [ "$LIBC_HOME/$1/$file" = "$LIBC_HOME/include/gnu/lib-names-64.h" ]    \
        || [ "$LIBC_HOME/$1/$file" = "$LIBC_HOME/include/regexp.h" ]              \
        || [ "$LIBC_HOME/$1/$file" = "$LIBC_HOME/include/sys/elf.h" ]             \
        || [ "$LIBC_HOME/$1/$file" = "$LIBC_HOME/include/sys/vm86.h" ]            \
        || [ "$LIBC_HOME/$1/$file" = "$LIBC_HOME/include/tgmath.h" ]; then
            # do not include these files
            continue
        else
            HDRS+=($LIBC_HOME/$1/$file)
            header=$1/$file
            header=${header#"include/"}
            echo "#include <$header>" >> $FILE
        fi
    done
}

get_header_files include
get_header_files include/neteconet
get_header_files include/netash
get_header_files include/netrom
get_header_files include/sys
get_header_files include/sys/platform
# get_header_files include/bits
# get_header_files include/bits/platform
# get_header_files include/bits/types
get_header_files include/net
get_header_files include/nfs
get_header_files include/netrose
get_header_files include/rpc
# get_header_files include/finclude
get_header_files include/netax25
get_header_files include/netpacket
get_header_files include/scsi
get_header_files include/netatalk
get_header_files include/arpa
get_header_files include/netiucv
get_header_files include/netipx
get_header_files include/gnu
get_header_files include/protocols

echo -n -e "\n\n" >> $FILE

# echo ${HDRS[@]}

OUTFILE=/tmp/libc_function_signatures

# get AST
# filter for function declarations
# filter out previous declarations
# remove color
# strip output
# remove implicit column
# remove used column
# remove scratch space column
# filter out repeated lines
if ! [ -f $OUTFILE ]; then
    clang -D_GNU_SOURCE -I$LIBC_HOME/include -Xclang -ast-dump -fsyntax-only ${HDRS[@]} \
        | grep 'FunctionDecl' \
        | grep -v 'prev 0x' \
        | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g' \
        | awk -v n=6 '{ for (i=n; i<=NF; i++) printf "%s%s", $i, (i<NF ? OFS : ORS)}' \
        | awk -v n=2 '{ if ($1 == "implicit") {for (i=n; i<=NF; i++) printf "%s%s", $i, (i<NF ? OFS : ORS)} else {print $0}}' \
        | awk -v n=2 '{ if ($1 == "used") {for (i=n; i<=NF; i++) printf "%s%s", $i, (i<NF ? OFS : ORS)} else {print $0}}' \
        | awk -v n=2 '{ if ($1 ~ /space>/) {for (i=n; i<=NF; i++) printf "%s%s", $i, (i<NF ? OFS : ORS)} else {print $0}}' \
        | awk '!seen[$0]++' \
        &> $OUTFILE
fi

python3 generate_libc_wrappers.py $OUTFILE

# TODO: generate tests that run libc wrappers and identify if they return pointers to domain 0

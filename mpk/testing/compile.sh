DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
PATH_TO_BIN=$DIR/bin
JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64

SRC_DIR=$DIR/gen-snippets
LIBRARIES="$DIR/common/libswscommon.a $DIR/erim/liberim.a $SRC_DIR/libhello.so"

# Iterate over every .c file inside the directory
for file in $(find "$SRC_DIR" -type f -name "*.c"); do
    name=$(basename "$file" .c)
    gcc -Wall -O2 -g -I. -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -fno-inline -shared -o "$PATH_TO_BIN/lib$name.so" "$file" -L"$SRC_DIR" -lhello -lm "$DIR/common/libswscommon.a" "$DIR/erim/liberim.a"
done

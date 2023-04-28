#JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64

cd gen-snippets
gcc -fPIC -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -shared -o libprint.so print.c -L. -lhello
cd ../output
java -Djava.library.path=../gen-snippets/ HelloJNI

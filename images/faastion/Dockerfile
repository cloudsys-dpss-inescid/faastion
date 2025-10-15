FROM ubuntu:22.04

ADD disk/ /

RUN apt update && \
    apt install libgomp1 libxml2-dev \
    patchelf gcc wget make gawk python3.11 \
    bison patch cmake build-essential \
    maven zip unzip zlib1g-dev -y

RUN ln -s /usr/bin/python3.11 /usr/bin/python3

# Download graalvm
RUN wget https://download.oracle.com/graalvm/17/archive/graalvm-jdk-17.0.7_linux-x64_bin.tar.gz && \
    tar -vzxf graalvm-jdk-17.0.7_linux-x64_bin.tar.gz && \
    rm graalvm-jdk-17.0.7_linux-x64_bin.tar.gz

# Extract libc
RUN tar xzvf glibc-2.35.tar.gz && \
    rm glibc-2.35.tar.gz

# Build patched libc
RUN cd glibc-2.35 && \
    patch -s -p1 < ../glibc-2.35.patch && \
    mkdir build && \
    cd build && \
    ../configure --prefix=$(pwd)/install && \
    make -j`nproc` && \
    make install -j`nproc`

# Download and extract igraph
RUN wget https://github.com/igraph/igraph/releases/download/0.10.15/igraph-0.10.15.tar.gz && \
    tar xzvf igraph-0.10.15.tar.gz && \
    rm igraph-0.10.15.tar.gz

# Build igraph
RUN cd igraph-0.10.15 && \
    mkdir build && \
    cd build && \
    cmake -DBUILD_SHARED_LIBS=1 .. && \
    cmake --build . && \
    cmake --install .

# Make the entrypoint script executable.
RUN chmod +x /start.sh

ENV ARGO_HOME=/faastion
ENV DEF_JAVA_HOME=/graalvm-jdk-17.0.7+8.1/
ENV JAVA_HOME=/graalvm-jdk-17.0.7+8.1/
ENV LIBC_HOME=/glibc-2.35/build/install/
ENV JAVASSIST_HOME=/faastion/native-execution/javassist/
ENV IGRAPH_HOME=/usr/local
ENV RESOURCES_DIR=/resources

# Required by polyglot-proxy
RUN ln -s /usr/lib/x86_64-linux-gnu/libz.so.1.2.11 /usr/lib/x86_64-linux-gnu/libz.so

RUN unzip faastion.zip && \
    rm faastion.zip && \
    cd faastion/graalvisor && \
    ./build.sh local

RUN cd faastion/native-execution/instrumentation && \
    ./build.sh

ENTRYPOINT ["/start.sh"]

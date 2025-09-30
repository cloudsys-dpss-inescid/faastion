FROM ubuntu:22.04

ARG libc_home

ADD disk/ /

RUN apt update && \
    apt install libgomp1 libxml2-dev patchelf gcc wget make gawk python3.11 bison patch cmake build-essential -y

RUN ln -s /usr/bin/python3.11 /usr/bin/python3

# Change polyglot-proxy loader
RUN patchelf --set-interpreter $libc_home/lib/ld-linux-x86-64.so.2 /graalvisor/build/native-image/polyglot-proxy

# Download and extract igraph
RUN wget https://github.com/igraph/igraph/releases/download/0.10.15/igraph-0.10.15.tar.gz && \
    tar xzvf igraph-0.10.15.tar.gz

# Build igraph
RUN cd igraph-0.10.15 && \
    mkdir build && \
    cd build && \
    cmake -DBUILD_SHARED_LIBS=1 .. && \
    cmake --build . && \
    cmake --install .

# Make the entrypoint script executable.
RUN chmod +x /graalvisor/start.sh

ENTRYPOINT ["/graalvisor/start.sh"]

FROM ubuntu:22.04

ARG build_home
ARG libc_home

ADD disk/ /

RUN apt update && \
    apt install libgomp1 libxml2-dev patchelf -y
    
RUN patchelf --set-interpreter $libc_home/lib/ld-linux-x86-64.so.2 /graalvisor/build/native-image/polyglot-proxy

RUN ln -s $build_home/libigraph.so.3 $build_home/libigraph.so

# Make the entrypoint script executable.
RUN chmod +x /graalvisor/start.sh

ENTRYPOINT ["/graalvisor/start.sh"]

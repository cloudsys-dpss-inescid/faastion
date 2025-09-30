FROM debian

ENV JAVA_HOME=/jvm
ENV HYDRA_PYTHON=/jvm/hydra-venv/bin/python

ADD disk/ /

RUN apt update && \
    apt install -y wget

# Detect the processor architecture and download the Minio client
# RUN ARCH=$(dpkg --print-architecture); \
#     if [ "$ARCH" = "amd64" ]; then \
#         wget https://dl.min.io/client/mc/release/linux-amd64/mc; \
#     elif [ "$ARCH" = "arm64" ]; then \
#         wget https://dl.min.io/client/mc/release/linux-arm64/mc; \
#     else \
#         echo "Unsupported architecture: $ARCH"; \
#         exit 1; \
#     fi

# RUN chmod +x mc && \
#     mv mc /usr/local/bin/mc

# Because of jvips
RUN apt -y install libgomp1 libexpat-dev liblcms2-dev libwebpmux3 libwebpdemux2 libwebp-dev libimagequant-dev libexif-dev libglibd-2.0-dev
# Because of python classify (pytorch)
# RUN apt -y install libopenmpi-dev
# Because of python thumbnail
# RUN apt -y install libopenjp2-7-dev

# Reconstruct venv symlinks inside the container.
#RUN rm -f /jvm/hydra-venv/bin/graalpy && \
#    ln -s /jvm/languages/python/bin/graalpy /jvm/hydra-venv/bin/graalpy

# Make the entrypoint script executable.
# RUN chmod +x /graalvisor

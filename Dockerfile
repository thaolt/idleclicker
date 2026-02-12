FROM alpine:latest

# Install MinGW cross-compiler and build tools
RUN apk add --no-cache \
    mingw-w64-gcc \
    make \
    git \
    bash

WORKDIR /work

CMD ["/bin/bash"]

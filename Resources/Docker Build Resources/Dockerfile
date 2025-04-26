# Base image
FROM ubuntu:22.04

# Metadata
LABEL maintainer="Miles Rupp"
LABEL description="Docker image for building and running ShompOS with GCC cross-compiler, GNU Assembler, GRUB, and QEMU, with X11 forwarding."

# Update and install required dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    bison \
    flex \
    libgmp3-dev \
    libmpc-dev \
    libmpfr-dev \
    texinfo \
    qemu-system-i386 \
    qemu-system-gui \
    grub-pc-bin \
    xorriso \
    libgtk-3-0 \
    curl \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Set environment variables for cross-compilation
ENV TARGET=i686-elf
ENV PREFIX=/usr/local/cross
ENV PATH="$PREFIX/bin:$PATH"
ENV DISPLAY=$DISPLAY
ENV QT_X11_NO_MITSHM=1

# Allow forwarding X11 sockets
VOLUME /tmp/.X11-unix

# Create necessary directories
WORKDIR /usr/src

# Install Binutils
RUN curl -LO http://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz && \
    tar -xzf binutils-2.40.tar.gz && \
    mkdir build-binutils && cd build-binutils && \
    ../binutils-2.40/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror && \
    make -j$(nproc) && make install && \
    cd .. && rm -rf binutils-2.40 build-binutils binutils-2.40.tar.gz

# Install GCC
RUN curl -LO http://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz && \
    tar -xzf gcc-12.2.0.tar.gz && \
    mkdir build-gcc && cd build-gcc && \
    ../gcc-12.2.0/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c,c++ --without-headers && \
    make -j$(nproc) all-gcc && make install-gcc && \
    make -j$(nproc) all-target-libgcc && make install-target-libgcc && \
    cd .. && rm -rf gcc-12.2.0 build-gcc gcc-12.2.0.tar.gz

# Verify installations
RUN $PREFIX/bin/$TARGET-gcc --version && $PREFIX/bin/$TARGET-ld --version

# Create workspace for your OS
WORKDIR /workspace


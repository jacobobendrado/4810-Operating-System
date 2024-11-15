# Makefile
# Set up configurations of running program
# Cedarville University 2024-25 OSDev Team

PATH := $(HOME)/opt/cross/bin:$(PATH)

# tools
CC = i686-elf-gcc
LD = i686-elf-ld
AS = i686-elf-as

# Flags
CFLAGS = -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -T src/linker.ld -nostdlib


BOOT=		src/kernel/boot.S
KERNEL=		src/kernel/kernel.c
LINKER=		src/linker.ld
KERNEL_OUT=	build/shompOS.bin
ISO_OUT=	build/shompOS.iso

all: build
build: clean
	mkdir -p build
	${AS} ${BOOT} -o build/boot.o
	${CC} ${CFLAGS} -c ${KERNEL} -o build/kernel.o
	${LD} ${LDFLAGS} -o ${KERNEL_OUT} build/boot.o build/kernel.o
run: build
	qemu-system-i386 -kernel ${KERNEL_OUT} -monitor stdio
build-iso: build
	mkdir -p build/iso/boot/grub
	cp src/grub.cfg build/iso/boot/grub
	cp ${KERNEL_OUT} build/iso/boot/grub
	grub-mkrescue -o ${ISO_OUT} build/iso
	rm -rf build/iso
run-iso: build-iso
	qemu-system-i386 -cdrom ${ISO_OUT}
clean:
	rm -rf build

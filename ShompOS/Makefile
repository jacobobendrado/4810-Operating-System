PATH := $(HOME)/opt/cross/bin:$(PATH)

# tools
CC = i686-elf-gcc
LD = i686-elf-ld
AS = i686-elf-as

# directories
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

LINKER_FILE = $(SRC_DIR)/linker.ld
KERNEL_OUT = $(BUILD_DIR)/shompOS.bin
ISO_OUT =	$(BUILD_DIR)/shompOS.iso

# Flags
CFLAGS = -ffreestanding -O2 -Wall -Wextra -m32 -I./inc -ggdb
LDFLAGS = -T $(LINKER_FILE) -nostdlib -o $(KERNEL_OUT)

KERNEL_OBJS = 	$(OBJ_DIR)/kernel.o \
				$(OBJ_DIR)/boot.o \
				$(OBJ_DIR)/heap.o \
				$(OBJ_DIR)/fake_libc.o

.PHONY: all build run run-curses clean

all: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/**/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/**/%.s
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@\

build: $(BUILD_DIR)/kernel.bin
	mkdir -p build/iso/boot/grub
	cp src/grub.cfg build/iso/boot/grub
	cp ${KERNEL_OUT} build/iso/boot/grub
	grub-mkrescue -o ${ISO_OUT} build/iso
	rm -rf build/iso

run: build
	qemu-system-i386 -cdrom ${ISO_OUT}
run-curses: build
	qemu-system-i386 -display curses -cdrom ${ISO_OUT}
debug: build
	qemu-system-i386 -s -S -cdrom $(ISO_OUT) &
	gdb $(KERNEL_OUT) -x debug.gdb
debug-curses: build
	qemu-system-i386 -display curses -s -S -cdrom $(ISO_OUT) 

clean:
	rm -rf build

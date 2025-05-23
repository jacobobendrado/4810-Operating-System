# Makefile
# Set up configurations of running program
# Cedarville University 2024-25 OSDev Team

PATH := $(HOME)/opt/cross/bin:$(PATH)

# tools
CC = i686-elf-gcc
LD = i686-elf-ld
AS = i686-elf-as

# directories
SRC_DIR = ./src
INC_DIR = ./inc
MNT_DIR = ./mnt
SCRIPTS_DIR = ./scripts
BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj
MNT_OUT_DIR = $(BUILD_DIR)/mnt

LINKER_FILE = $(SRC_DIR)/linker.ld
KERNEL_OUT = $(BUILD_DIR)/shompOS.bin
ISO_OUT = $(BUILD_DIR)/shompOS.iso

# Flags
# CFLAGS needs to know the subdirectories for includes
CFLAGS = -ffreestanding -O2 -Wall -Wextra -m32 -I$(INC_DIR) -I$(INC_DIR)/elf -I$(INC_DIR)/fake_libc -I$(INC_DIR)/fs -I$(INC_DIR)/process -I$(INC_DIR)/IO -I$(INC_DIR)/kernel -I$(INC_DIR)/memory -ggdb
LDFLAGS = -T $(LINKER_FILE) -nostdlib -o $(KERNEL_OUT)

KERNEL_OBJS = 	$(OBJ_DIR)/kernel.o \
				$(OBJ_DIR)/boot.o \
				$(OBJ_DIR)/heap.o \
				$(OBJ_DIR)/string.o \
				$(OBJ_DIR)/ramfs.o \
				$(OBJ_DIR)/ramfs_executables.o \
				$(OBJ_DIR)/fake_libc.o \
				$(OBJ_DIR)/process.o \
				$(OBJ_DIR)/context_switch.o \
				$(OBJ_DIR)/syscalls.o \
				$(OBJ_DIR)/elf.o \
				$(OBJ_DIR)/mnt.o

.PHONY: all build run run-curses debug debug-curses clean

all: $(BUILD_DIR)/shompOS.bin

$(BUILD_DIR)/shompOS.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/**/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/**/%.S
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/mnt.o: $(OBJ_DIR)/mnt.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o "$@.temp"
	$(CC) -r "$@.temp" $(wildcard $(MNT_OUT_DIR)/*) -o $@
	rm "$@.temp"

$(OBJ_DIR)/mnt.c:
	mkdir -p $(MNT_OUT_DIR)
	mkdir -p $(OBJ_DIR)
	$(SCRIPTS_DIR)/mnt_setup.sh $(MNT_DIR) $(LD) $(MNT_OUT_DIR) $(SCRIPTS_DIR) $@

build: $(BUILD_DIR)/shompOS.bin
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

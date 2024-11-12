PATH := $(HOME)/opt/cross/bin:$(PATH)

# Compiler and linker
CC = i686-elf-gcc
LD = i686-elf-ld

# Flags
CFLAGS = -ffreestanding -O2 -Wall -Wextra
CFLAGS_DEBUG = -ggdb $(CFLAGS)
LDFLAGS = -T src/linker.ld -nostdlib

# Directories
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = iso
BUILD_DEBUG_DIR = build_debug
ISO_DEBUG_DIR = iso_debug

# Files
KERNEL_OBJS = $(BUILD_DIR)/kernel.o $(BUILD_DIR)/boot.o
KERNEL_OBJS = $(BUILD_DEBUG_DIR)/kernel.o $(BUILD_DEBUG_DIR)/boot.o

.PHONY: all clean run iso iso_debug run_debug

all: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -lgcc

$(BUILD_DIR)/%.o: $(SRC_DIR)/kernel/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/kernel/%.s
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

iso: clean $(BUILD_DIR)/kernel.bin
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/kernel.bin
	cp $(SRC_DIR)/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_DIR)/shompos.iso $(ISO_DIR)

run: iso
	qemu-system-i386 -cdrom $(ISO_DIR)/shompos.iso

$(BUILD_DEBUG_DIR)/kernel.bin: $(KERNEL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -lgcc

$(BUILD_DEBUG_DIR)/%.o: $(SRC_DIR)/kernel/%.c
	mkdir -p $(BUILD_DEBUG_DIR)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

$(BUILD_DEBUG_DIR)/%.o: $(SRC_DIR)/kernel/%.s
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

iso_debug: clean $(BUILD_DEBUG_DIR)/kernel.bin
	mkdir -p $(ISO_DEBUG_DIR)/boot/grub
	cp $(BUILD_DEBUG_DIR)/kernel.bin $(ISO_DEBUG_DIR)/boot/kernel.bin
	cp $(SRC_DIR)/grub.cfg $(ISO_DEBUG_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_DEBUG_DIR)/shompos.iso $(ISO_DEBUG_DIR)

run_debug: iso_debug
	qemu-system-i386 -s -S -cdrom $(ISO_DEBUG_DIR)/shompos.iso &
	gdb $(ISO_DEBUG_DIR)/boot/kernel.bin -x debug.gdb

clean:
	rm -rf $(BUILD_DIR) $(BUILD_DEBUG_DIR) $(ISO_DIR) $(ISO_DEBUG_DIR)

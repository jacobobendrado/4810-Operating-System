# Compiler and linker
CC = i686-elf-gcc
LD = i686-elf-ld

# Flags
CFLAGS = -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -T src/linker.ld -nostdlib

# Directories
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = iso

# Files
KERNEL_OBJS = $(BUILD_DIR)/kernel.o $(BUILD_DIR)/boot.o

.PHONY: all clean run iso

all: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -lgcc

$(BUILD_DIR)/%.o: $(SRC_DIR)/kernel/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/kernel/%.s
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

iso: $(BUILD_DIR)/kernel.bin
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/kernel.bin
	cp $(SRC_DIR)/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_DIR)/shompos.iso $(ISO_DIR)

run: iso
	qemu-system-i386 -cdrom $(ISO_DIR)/shompos.iso

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)

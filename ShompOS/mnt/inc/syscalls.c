// mnt/inc/syscalls.c
// Library to wrap syscalls
// Cedarville University 2024-25 OSDev Team

/*
COMPILATION INSTRUCTIONS:
To compile a source file called "source_file.c" in "mnt/source_dir" into "exe_file", follow these commands

~/opt/cross/bin/i686-elf-gcc -ffreestanding -nostartfiles  -m32 -fPIE -c -o syscalls.o syscalls.c
~/opt/cross/bin/i686-elf-gcc -ffreestanding -nostartfiles  -m32 -fPIE -c -o assembly.o syscalls.S
cd ../<source_dir>
~/opt/cross/bin/i686-elf-gcc -ffreestanding -nostartfiles  -m32 -fPIE -c -o <source_file.o> <source_file.c>
~/opt/cross/bin/i686-elf-gcc -ffreestanding -nostartfiles -nostdlib -m32 -Wl,-emain -o <exe_file> <source_file>.o ../inc/syscalls.o ../inc/assembly.o
*/


#include "syscalls.h"

extern uint32_t do_syscall(const uint32_t eax, const uint32_t ebx, const uint32_t ecx, const uint32_t edx, const uint32_t esi, const uint32_t edi, const uint32_t ebp);

void exit(int32_t error_code) {
    do_syscall(1, error_code, 0, 0, 0, 0, 0);
}

uint32_t read(uint32_t fd, char *buf, uint32_t count) {
    do_syscall(3, fd, (uint32_t)buf, count, 0, 0, 0);
}

uint32_t write(uint32_t fd, const char *buf, uint32_t count) {
    do_syscall(4, fd, (uint32_t)buf, count, 0, 0, 0);
}

uint32_t open(const char *filename, int flags, uint32_t mode) {
    do_syscall(5, (uint32_t)filename, flags, mode, 0, 0, 0);
}

uint32_t close(uint32_t fd) {
    do_syscall(6, fd, 0, 0, 0, 0, 0);
}

void putchar(char c, uint8_t color, uint32_t x, uint32_t y) {
    do_syscall(7, c, color, x, y, 0, 0);
}
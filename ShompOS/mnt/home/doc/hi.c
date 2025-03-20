// ~/opt/cross/bin/i686-elf-gcc -ffreestanding -nostartfiles -nostdlib -m32 -fPIE -c -o hi.o hi.c

#include "greeting.h"

void hi() {
    terminal_writestring("hi\n");
}
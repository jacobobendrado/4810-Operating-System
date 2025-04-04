// ~/opt/cross/bin/i686-elf-gcc -ffreestanding -nostartfiles -nostdlib -m32 -Wl,-emain -o greeting hello.o hi.o

#ifndef GREETING_H
#define GREETING_H

#include <stddef.h>
#include <stdint.h>


void terminal_writestring(const char *data);
void hello();
void hi();

#endif
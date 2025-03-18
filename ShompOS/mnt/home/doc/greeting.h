// ~/opt/cross/bin/i686-elf-gcc -ffreestanding -nostartfiles -nostdlib -m32 -Wl,-emain -o greeting hi.o hello.o

#ifndef GREETING_H
#define GREETING_H

#include <stddef.h>
#include <stdint.h>


void terminal_writestring(const char *data);
void terminal_putchar(char c, uint8_t *terminal_color, size_t *terminal_row, uint16_t *terminal_column, uint16_t** terminal_buffer);
void hello();
void hi();

#endif

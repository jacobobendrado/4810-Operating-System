// ~/opt/cross/bin/i686-elf-gcc -ffreestanding -nostartfiles -nostdlib -m32 -c -o hi.o hi.c

#include "greeting.h"

void hi(uint8_t *terminal_color, size_t *terminal_row, uint16_t *terminal_column, uint16_t** terminal_buffer) {
    terminal_putchar('H', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('i', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('.', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('\n', terminal_color, terminal_row, terminal_column, terminal_buffer);
}

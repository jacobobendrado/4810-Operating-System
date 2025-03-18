// ~/opt/cross/bin/i686-elf-gcc -ffreestanding -nostartfiles -nostdlib -m32 -c -o hello.o hello.c

#include "greeting.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

// ----- Bare Bones -----
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};


static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}


static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y, uint16_t* terminal_buffer)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_advance_row(size_t *terminal_row, uint16_t *terminal_column)
{
	if (++(*terminal_row) == VGA_HEIGHT){
		terminal_row = 0;
	}

	*terminal_column = 0;
}


void terminal_putchar(char c, uint8_t *terminal_color, size_t *terminal_row, uint16_t *terminal_column, uint16_t** terminal_buffer)
{
    if (c == '\n') {
		terminal_advance_row(terminal_row, terminal_column);
	} else {
		terminal_putentryat(c, *terminal_color, *terminal_column, *terminal_row, *terminal_buffer);
		if (++(*terminal_column) == VGA_WIDTH) {
			*terminal_column = 0;
			if (++(*terminal_row) == VGA_HEIGHT)
				*terminal_row = 0;
		}
	}
}

void terminal_clear(uint8_t *terminal_color, size_t *terminal_row, uint16_t *terminal_column, uint16_t** terminal_buffer) {
	for (uint8_t y = 0; y < VGA_HEIGHT; y++){
		for (uint8_t x = 0; x < VGA_WIDTH; x++){

			terminal_putentryat(' ', *terminal_color, x, y, *terminal_buffer);
		}
	}
	*terminal_column = 0;
	*terminal_row = 0;
}

int main() {
    uint8_t terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	size_t terminal_row = 0;
	uint16_t terminal_column = 0;
	uint16_t* terminal_buffer = (uint16_t*) 0xB8000;

	terminal_clear(&terminal_color, &terminal_row, &terminal_column, &terminal_buffer);

    hello(&terminal_color, &terminal_row, &terminal_column, &terminal_buffer);
    hi(&terminal_color, &terminal_row, &terminal_column, &terminal_buffer);
    return 1;
}

void hello(uint8_t *terminal_color, size_t *terminal_row, uint16_t *terminal_column, uint16_t** terminal_buffer) {
    terminal_putchar('H', terminal_color, terminal_row, terminal_column, terminal_buffer);
	terminal_putchar('e', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('l', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('l', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('o', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar(' ', terminal_color, terminal_row, terminal_column, terminal_buffer);
	terminal_putchar('W', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('o', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('r', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('l', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('d', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('!', terminal_color, terminal_row, terminal_column, terminal_buffer);
    terminal_putchar('\n', terminal_color, terminal_row, terminal_column, terminal_buffer);
}

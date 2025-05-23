// ~/opt/cross/bin/i686-elf-gcc -ffreestanding -nostartfiles -nostdlib -m32 -fPIE -c -o hello.o hello.c

#include "greeting.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

// The option -fPIE will lead to global variables being loaded at places offset by a table, so 
// we need to make sure that they go there instead of the .bss section. More research might help
// determine how to make .bss section dynamic as well. Initializing them puts them in .data
// instead of .bss.
size_t terminal_row = -1;
size_t terminal_column = -1;
uint8_t terminal_color = -1;
uint16_t* terminal_buffer = 0xffff;

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

size_t strlen(const char *s) {
    size_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

void init_terminal(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_clear() {
	for (uint8_t y = 0; y < VGA_HEIGHT; y++){
		for (uint8_t x = 0; x < VGA_WIDTH; x++){
			terminal_putentryat(' ', terminal_color, x, y);
		}
	}
	terminal_column = 0;
	terminal_row = 0;
}

void terminal_advance_row()
{
	if (++terminal_row == VGA_HEIGHT){
		terminal_row = 0;
		terminal_clear();
	}

	terminal_column = 0;
}

void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void terminal_putchar(char c)
{
    if (c == '\n') {
		terminal_advance_row();
	} else {
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) {
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
				terminal_row = 0;
		}
	}
}

void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		if (data[i] == 0x0A) {
			terminal_advance_row();
		} else {
			terminal_putchar(data[i]);
		}
}

void terminal_writestring(const char* data)
{
	terminal_write(data, strlen(data));
}

int main() {
    init_terminal();
    hello();
    hi();
    return 1;
}

void hello() {
    terminal_writestring("Hello!\n");
}
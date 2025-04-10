#pragma once

#include <stdint.h>
#include <stddef.h>
#include <ramfs.h>
#include <ramfs_executables.h>

// --- output ---
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

extern size_t terminal_row;
extern size_t terminal_column;
extern uint8_t terminal_color;
extern uint16_t* terminal_buffer;
extern ramfs_dir_t* current_dir;
extern ramfs_dir_t* system_root;

typedef struct _KEY_state {
	uint8_t unused : 4;
    uint8_t caps: 1;
	uint8_t alt : 1;
	uint8_t ctrl : 1;
	uint8_t shift : 1;
} KEY_state;

typedef enum {
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
} vga_color;

void init_kb();
void init_terminal(void);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(char c);
void terminal_writeint(int number);
void terminal_writestring(const char* data);
void terminal_clear();
void terminal_main();

inline uint8_t vga_entry_color(vga_color fg, vga_color bg)
{
	return fg | bg << 4;
}

inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

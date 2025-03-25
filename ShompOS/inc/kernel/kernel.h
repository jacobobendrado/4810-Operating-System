#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <ramfs.h>

typedef struct _IDT_pointer {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) IDT_pointer;

typedef struct _IDT_entry {
	uint16_t offset_lowerbits;
	uint16_t selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_upperbits;
} __attribute__((packed)) IDT_entry;

typedef struct _KEY_state {
	uint8_t unused : 4;
  uint8_t caps: 1;
	uint8_t alt : 1;
	uint8_t ctrl : 1;
	uint8_t shift : 1;
} KEY_state;

void terminal_writestring(const char* data);
void terminal_writeint(int number);
void terminal_clear();

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
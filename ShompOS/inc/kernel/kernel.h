// kernel.h
// Core functionality for the kernel
// Cedarville University 2024-25 OSDev Team

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <ramfs.h>

// IO Ports for PICs
#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT 0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT 0xA1

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

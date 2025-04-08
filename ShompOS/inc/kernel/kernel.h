#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <ramfs.h>


// IDT_SIZE: Specific to x86 architecture
#define IDT_SIZE 0x100
// EXCEPTIONS_SIZE: Number of exceptions that are used in IDT
#define EXCEPTIONS_SIZE 0x20
// KERNEL_CODE_SEGMENT_OFFSET: the first segment after the null segment in gdt.s
#define KERNEL_CODE_SEGMENT_OFFSET 0x8
// 32-bit Interrupt gate: 0x8E
// ( P=1, DPL=00b, S=0, type=1110b => type_attr=1000_1110b=0x8E)
// (see https://wiki.osdev.org/Interrupt_Descriptor_Table#Structure_on_IA-32)
#define IDT_INTERRUPT_GATE_32BIT 0x8E
// 32-bit Trap gate: 0x8F
// ( P=1, DPL=00b, S=0, type=1111b => type_attr=1000_1111b=0x8:wF)
#define IDT_TRAP_GATE_32BIT 0x8F
// IO Ports for PICs
#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT 0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT 0xA1
// IO Ports for Keyboard
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
/// IO Ports for PIT
#define PIT_CHANNEL_0_DATA_PORT 0x40
#define PIT_COMMAND_MODE_PORT 0x43
// PIT will trigger an interrupt at a rate of 1193180 / divisor Hz
// 0xFFFF results in around 18.3 Hz, the slowest possible with 16 bits
#define PIT_DIVISOR 0x0FFF

// ----- experimental attempt to run commands
#define CMD_MAX_LEN 64


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


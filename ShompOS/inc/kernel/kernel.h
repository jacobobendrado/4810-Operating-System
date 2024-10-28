#pragma once

// ----- Assembly functions -----
extern void load_gdt();
extern void keyboard_handler();
extern void int_zero_handler();
extern char ioport_in(uint16_t port);
extern void ioport_out(uint16_t port, uint8_t data);
extern void load_idt(uint32_t* idt_address);
extern void enable_interrupts();
extern void* isr_stub_table[];



// ----- Structs -----
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
	uint8_t unused : 5;
	uint8_t alt : 1;
	uint8_t ctrl : 1;
	uint8_t shift : 1;
} KEY_state;

void terminal_writestring(const char* data);

size_t strlen(const char* str);
void* memset(void* bufptr, int value, size_t size);
void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size);
int memcmp(const void* aptr, const void* bptr, size_t size);
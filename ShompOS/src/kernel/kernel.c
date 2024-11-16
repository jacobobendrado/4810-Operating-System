// IDT_SIZE: Specific to x86 architecture
#define IDT_SIZE 256
// EXCEPTIONS_SIZE: Number of exceptions that are used in IDT
#define EXCEPTIONS_SIZE 32
// KERNEL_CODE_SEGMENT_OFFSET: the first segment after the null segment in gdt.s
#define KERNEL_CODE_SEGMENT_OFFSET 0x8
// 32-bit Interrupt gate: 0x8E
// ( P=1, DPL=00b, S=0, type=1110b => type_attr=1000_1110b=0x8E) 
// (see https://wiki.osdev.org/Interrupt_Descriptor_Table#Structure_on_IA-32)
#define IDT_INTERRUPT_GATE_32BIT 0x8E
// 32-bit Trap gate: 0x8F
// ( P=1, DPL=00b, S=0, type=1111b => type_attr=1000_1111b=0x8F)
#define IDT_TRAP_GATE_32BIT 0x8F
// IO Ports for PICs
#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT 0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT 0xA1
// IO Ports for Keyboard
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// ----- Includes -----
#include <kernel/kernel.h>
#include <kernel/boot.h>
#include <fake_libc/fake_libc.h>
#include <IO/keyboard_map.h>
#include <memory/heap.h>


// ----- Global variables -----
// This is our entire IDT. Room for 256 interrupts
IDT_entry IDT[IDT_SIZE]; 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
KEY_state special_key_state = {0,0,0,0};

// ----- debugging/example variables -----
void* ptr[10];

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


void init_terminal(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
	char* term = "shompOS>";
	terminal_writestring(term);
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

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c)
{
    if (c == '\n') {
		char* term = "shompOS>";
		terminal_advance_row();
		terminal_writestring(term);
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

void terminal_clear() {
	for (uint8_t y = 0; y < VGA_HEIGHT; y++){
		for (uint8_t x = 0; x < VGA_WIDTH; x++){
			terminal_putentryat(' ', terminal_color, x, y);
		}	
	}
}

void exception_handler() {
	terminal_writestring("Exception!");
	__asm__ volatile ("cli; hlt");
}

void idt_set_descriptor(uint8_t interrupt_num, void* offset, uint8_t flags) {
	IDT[interrupt_num].offset_lowerbits = (uint32_t)offset & 0x0000FFFF;
	IDT[interrupt_num].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[interrupt_num].zero = 0;
	IDT[interrupt_num].type_attr = flags;
	IDT[interrupt_num].offset_upperbits = ((uint32_t)offset & 0xFFFF0000) >> 16;
}

// ----- PageKey Video -----
void init_idt() {
	unsigned int offset;

	offset = (unsigned int)int_zero_handler;
	idt_set_descriptor(0x00, (void*)offset,  IDT_TRAP_GATE_32BIT);

	for (int i = 1; i < EXCEPTIONS_SIZE; i++) {
		idt_set_descriptor(i, isr_stub_table[i], IDT_TRAP_GATE_32BIT);
	}

	offset = (unsigned int)keyboard_handler;
	idt_set_descriptor(0x21, (void*)offset,  IDT_INTERRUPT_GATE_32BIT);

	// the PICs (programmable interrupt controler)
	// must be initialized before use. this can be done
	// by sending magic values (initialization command words)
	// to their I/O ports. 

	// ICW1: begin PIC initialization
	// by sending 0x11 to the command ports, the PICs
	// will begin waiting for next 3 ICWs at their 
	// data ports
	ioport_out(PIC1_COMMAND_PORT, 0x11);
	ioport_out(PIC2_COMMAND_PORT, 0x11);
	
	// ICW2: vector offset
	// each PIC has 8 interrupts, starting from a
	// specified offset.
	// interrupts 0x00..0x20 (00..31d) -> reserved by Intel
	// interrupts 0x20..0x27 (30..39d) -> PIC1
	// interrupts 0x28..0x2F (40..48d) -> PIC2
	// see https://wiki.osdev.org/8259_PIC#Programming_with_the_8259_PIC
	ioport_out(PIC1_DATA_PORT, 0x20);
	ioport_out(PIC2_DATA_PORT, 0x28);

	// ICW3: cascading (how each PIC relates to the other)
	// tell PIC1 that PIC2 is at IRQ2 (0000 0100)
	// tell PIC2 its cascade identity (0000 0010)
	// see https://wiki.osdev.org/8259_PIC#Code_Examples
	ioport_out(PIC1_DATA_PORT, 0x4);
	ioport_out(PIC2_DATA_PORT, 0x2);

	// ICW4: environment information
	// sending a 1 tells the PICs to use 8086 mode.
	// i'm blindly trusting the wiki on this one
	ioport_out(PIC1_DATA_PORT, 0x1);
	ioport_out(PIC2_DATA_PORT, 0x1);


	ioport_out(PIC1_DATA_PORT, 0xFF);
	ioport_out(PIC2_DATA_PORT, 0xFF);
	
	// create a pointer to our idt, this is 
	// required to be the format of limit (size
	// in bytes - 1) and base (starting address)
	IDT_pointer idt_ptr;
	idt_ptr.limit = (sizeof(IDT_entry) *IDT_SIZE) - 1;
	idt_ptr.base = (uint32_t) &IDT;

	// pass address to load_idt (defined in boot.s)
	// where it will be loaded in the IDT register
	load_idt((uint32_t*) &idt_ptr);
}

void init_kb() {
	ioport_out(PIC1_DATA_PORT, 0xFD);
}

void handle_keyboard_interrupt() {
	// clear interrupt; tells PIC we
	// are handling it.
	ioport_out(PIC1_COMMAND_PORT, 0x20);
	unsigned char status = ioport_in(KEYBOARD_STATUS_PORT);
	
	if (status & 0x1) {
		
		// the PIC will hand us an 8-bit value. bits 0..6 
		// represent the key pressed, this is NOT an ascii 
		// value. bit 7 is 0 if the key has just been pressed,
		// 1 if the key has just been released.  
		char keycode = ioport_in(KEYBOARD_DATA_PORT);
		
		// set shift flag based on keycode MSB 
		if (keycode == 42 || (uint8_t)keycode == 170 ||
			keycode == 54 || (uint8_t)keycode == 182){
			special_key_state.shift = 1 - ((uint8_t)keycode >> 7);
		} 
		
		// ignore key releases
		else if ((uint8_t)keycode > 127) return;

		// execute div by 0 exception on "0" press
		else if (keyboard_map[(uint8_t) keycode] == '0') {
			int temp;
			__asm__ volatile (
			  "div %1\n\t"
			  : "=a" (temp)
			  : "r" (0), "a" (1)
			  : "cc");
		}



		else if (keyboard_map[(uint8_t) keycode] == 'a') {
			free(&ptr[1]);
		} else if (keyboard_map[(uint8_t) keycode] == 'q') {
			free(&ptr[2]);
		} else if (keyboard_map[(uint8_t) keycode] == 'w') {
			free(&ptr[3]);
		} else if (keyboard_map[(uint8_t) keycode] == 'e') {
			free(&ptr[4]);
		} else if (keyboard_map[(uint8_t) keycode] == 'r') {
			free(&ptr[5]);
		} else if (keyboard_map[(uint8_t) keycode] == 't') {
			free(&ptr[6]);
		} else if (keyboard_map[(uint8_t) keycode] == 'y') {
			free(&ptr[7]);
		} else if (keyboard_map[(uint8_t) keycode] == 'u') {
			free(&ptr[8]);
		} else if (keyboard_map[(uint8_t) keycode] == 'i') {
			free(&ptr[9]);
		} 

		// simple allocation routine
		else if (keyboard_map[(uint8_t) keycode] == '1') {
			ptr[1] = allocate(1);
		} else if (keyboard_map[(uint8_t) keycode] == '2') {
			ptr[2] = allocate(8);
		} else if (keyboard_map[(uint8_t) keycode] == '3') {
			ptr[3] = allocate(24);
		} else if (keyboard_map[(uint8_t) keycode] == '4') {
			ptr[4] = allocate(1);
		} else if (keyboard_map[(uint8_t) keycode] == '5') {
			ptr[5] = allocate(8);
		} else if (keyboard_map[(uint8_t) keycode] == '6') {
			ptr[6] = allocate(24);
		} else if (keyboard_map[(uint8_t) keycode] == '7') {
			ptr[7] = allocate(56);
		} else if (keyboard_map[(uint8_t) keycode] == '8') {
			ptr[8] = allocate(120);
		} else if (keyboard_map[(uint8_t) keycode] == '9') {
			ptr[9] = allocate(248);
		} 

		
		else if (keyboard_map[(uint8_t) keycode] == 'o') {
			terminal_advance_row();
		} 
		else if (keyboard_map[(uint8_t) keycode] == 'p') {
			print_free_counts();
		} 
		
		
		// print character with SHFT modification
		else {
			terminal_putchar(keyboard_map[(uint8_t) keycode] - (special_key_state.shift * 32));
		}
	}
}

void handle_div_by_zero() {
	terminal_writestring("Div by zero!");
}


// ----- Entry point -----
void kernel_main() {
    init_terminal();
	init_idt();
	init_kb();
	init_heap(HEAP_LOWER_BOUND);
	enable_interrupts();
	while(1);
}
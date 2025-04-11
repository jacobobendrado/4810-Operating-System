// kernel.c
// Core functionality for the kernel
// Cedarville University 2024-25 OSDev Team

// ----- Includes -----
#include <kernel/kernel.h>
#include <kernel/boot.h>

#include <fake_libc/fake_libc.h> // Is this still relevant?

#include <memory/heap.h>

#include <process/process.h>

#include <string.h>
#include <ramfs.h>
#include <ramfs_executables.h>
#include <elf.h>
#include <tty.h>

// ----- Constants ----
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
/// IO Ports for PIT
#define PIT_CHANNEL_0_DATA_PORT 0x40
#define PIT_COMMAND_MODE_PORT 0x43
// PIT will trigger an interrupt at a rate of 1193180 / divisor Hz
// 0xFFFF results in around 18.3 Hz, the slowest possible with 16 bits
#define PIT_DIVISOR 0x0FFF

// ----- experimental attempt to run commands
#define CMD_MAX_LEN 64

// ----- Global variables -----
// This is our entire IDT. Room for 256 interrupts
IDT_entry IDT[IDT_SIZE];

// ----- debugging/example variables -----
bool memory_mode = false;
bool input_mode = false;
uint32_t input_len = 0;
char* input_ptr = NULL;
uint8_t alloc_size = 0;
void* ptr[10];



void kill_process_exception() {
	__asm__ volatile ("cli; hlt");
}

// void exception_handler(uint8_t num) {
void exception_handler() {
	char* str = "Exception!";
	ramfs_write(STDOUT_FILENO, str, strlen(str));

	// char c[3] = {(char)num+65, ' ', '\0'};
	// terminal_writestring(c);
	kill_process_exception();
}

void idt_set_descriptor(uint8_t interrupt_num, void* offset, uint8_t flags) {
	IDT[interrupt_num].offset_lowerbits = (uint32_t)offset & 0x0000FFFF;
	IDT[interrupt_num].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[interrupt_num].zero = 0;
	IDT[interrupt_num].type_attr = flags;
	IDT[interrupt_num].offset_upperbits = ((uint32_t)offset & 0xFFFF0000) >> 16;
}

void init_pit(uint32_t divisor) {
    // Command byte: Channel 0, low/high byte, rate generator mode
    ioport_out(PIT_COMMAND_MODE_PORT, 0x36);

    // Send divisor (low byte then high byte)
    ioport_out(PIT_CHANNEL_0_DATA_PORT, divisor & 0xFF);
    ioport_out(PIT_CHANNEL_0_DATA_PORT, (divisor >> 8) & 0xFF);

	// Enable IRQ0 in the PIC
    ioport_out(PIC1_DATA_PORT, ioport_in(0x21) & ~(1 << 0));
}

void handle_clock_interrupt() {
	// clear interrupt; tells PIC we
	// are handling it.
	ioport_out(PIC1_COMMAND_PORT, 0x20);

	// terminal_writestring("clock");
	switch_process_from_queue();

}
// ----- PageKey Video -----
void init_idt() {
	for (int i = 0; i < EXCEPTIONS_SIZE; i++) {
		idt_set_descriptor(i, isr_stub_table[i], IDT_TRAP_GATE_32BIT);
	}

	idt_set_descriptor(0x80, (void*)syscall_handler,  IDT_INTERRUPT_GATE_32BIT);
	idt_set_descriptor(0x21, (void*)keyboard_handler,  IDT_INTERRUPT_GATE_32BIT);
	idt_set_descriptor(0x20, (void*)clock_handler, IDT_TRAP_GATE_32BIT);


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


void handle_div_by_zero() {
	char* str = "Div by Zero!";
	ramfs_write(STDOUT_FILENO, str, strlen(str));
}

// ===== SAMPLE PROCESSES =====
void sample() {
	uint8_t row = 0;
	uint8_t col = 0;
	while(1){
		uint8_t color = vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
		char buf[5] = "text";

		for (uint8_t i = 0; i < VGA_HEIGHT+4; i++){
			terminal_putentryat(' ', color, i, row);
		}
		for (uint8_t i = 0; i < 4 ; i++){
			terminal_putentryat(buf[i], color, i+col, row);
		}

		if (++row == VGA_HEIGHT) {
			row = 0;
			if (++col == VGA_HEIGHT) col = 0;
		}
		for (uint32_t i = 0xFFFFFF; i > 0; i-- );
	}
}

void sample2() {
	uint8_t color = 0;
	uint8_t row = 0;

	while (1) {
    	char buf[5];
    	addr_to_string(buf, (uintptr_t)color);

		for (uint8_t i = 4; i > 0 ; i--){
			terminal_putentryat(buf[4-i], color, VGA_WIDTH-i, row);
		}
		color++;
		if (++row == VGA_HEIGHT) row = 0;
		for (uint32_t i = 0xFFFFFFF; i > 0; i-- );
	}
}

void sample3() {
	uint8_t color = 0;
	uint8_t row = VGA_HEIGHT;

	while (1) {
    	char buf[8] = "       ";
    	// addr_to_string(buf, (uintptr_t)color);

		for (uint8_t i = 7; i > 0 ; i--){
			terminal_putentryat(buf[i], color, (VGA_WIDTH/2)+i, row);
		}
		if (--row == (uint8_t)-1) {
			row = VGA_HEIGHT;
			color += 0x10;
		}
		for (uint32_t i = 0xFFFFFF; i > 0; i-- );
	}
}

void test_jump() {
	char* str = "before wait\n";
	ramfs_write(STDOUT_FILENO, str, strlen(str));
    for (uint32_t i = 0x00FFFFFF; i > 0; i-- ){
        uint8_t color = i;
        terminal_putentryat('X', color, i%5, terminal_row);
    }
	str = "\nafter wait. returning...!";
	ramfs_write(STDOUT_FILENO, str, strlen(str));
}
// ===== END SAMPLE PROCESSES =====

void terminal_backstop() {
    terminal_clear();
    char* str = "  <- if this guy is blinking, all other processes have exited.";
	ramfs_write(STDOUT_FILENO, str, strlen(str));
    uint8_t colors[2];
    colors[0] = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    colors[1] = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_BLACK);

    uint8_t idx = 0;
    while(1){
		char* str = "";
		ramfs_write(STDOUT_FILENO, str, strlen(str));
        terminal_putentryat('X', colors[idx], 0, 0);
        idx = (idx+1)%2;
        for (uint32_t i = 0x1FFFFFFF; i > 0; i--);
    };
}

// ----- Entry point -----
void init_shell(ramfs_dir_t* root) {
    current_dir = root;
}

void kernel_main() {
    init_terminal();
  	init_idt();
  	init_kb();
  	init_heap(HEAP_LOWER_BOUND);
  	enable_interrupts();
    ramfs_init_fd_system();
    ramfs_dir_t* root = system_root = init_fs();
    current_dir = root;
    init_stdio(root);

    if (!current_dir) {
        char* str = "Failed to initialize filesystem.";
		ramfs_write(STDOUT_FILENO, str, strlen(str));
        return;
    }

    // Initial terminal prompt
	char* str = "ShompOS initialized successfully!\n" 
				"Type 'help' for available commands\n"
				"shompOS> ";
	ramfs_write(STDOUT_FILENO, str, strlen(str));


    init_process(&terminal_backstop, allocate(500));
    init_process(&sample2, allocate(500));
    // init_process(&sample3, allocate(500));
    // init_process(&test_jump, allocate(500));
    init_process(&terminal_main, allocate(500));

    init_pit(PIT_DIVISOR);
    enable_interrupts();

    // Main kernel loop
    while(1);
}

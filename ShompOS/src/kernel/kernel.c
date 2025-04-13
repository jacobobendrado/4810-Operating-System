// kernel.c
// Core functionality for the kernel
// Cedarville University 2024-25 OSDev Team

// ----- Includes -----
#include <kernel.h>
#include <boot.h>
#include <heap.h>
#include <process.h>
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

// ----- Global variables -----
// This is our entire IDT. Room for 256 interrupts
IDT_entry IDT[IDT_SIZE];


// purpose: halt system when an exception occurs 
void exception_handler() {
	char* str = "Exception!";
	ramfs_write(STDOUT_FILENO, str, strlen(str));
	__asm__ volatile ("cli; hlt");
}

// alternate version that prints exception number
// void exception_handler(uint8_t num) {
// 	char* str = "Exception!";
// 	ramfs_write(STDOUT_FILENO, str, strlen(str));

// 	char c[3] = {(char)num+65, ' ', '\0'};
// 	ramfs_write(STDOUT_FILENO, c, 3);
// 	__asm__ volatile ("cli; hlt");
// }

// purpose: handles specifically the Div By 0 exception
void handle_div_by_zero() {
	char* str = "Div by Zero!";
	ramfs_write(STDOUT_FILENO, str, strlen(str));
}

// purpose: initializes the Programmable Interrupt Timer (PIT) to allow timed
//			interrupts, used to switch processes.
// divisor: PIT will trigger an interrupt at a rate of 1193180 / divisor Hz
void init_pit(uint32_t divisor) {
    // Command byte: Channel 0, low/high byte, rate generator mode
    ioport_out(PIT_COMMAND_MODE_PORT, 0x36);

    // Send divisor (low byte then high byte)
    ioport_out(PIT_CHANNEL_0_DATA_PORT, divisor & 0xFF);
    ioport_out(PIT_CHANNEL_0_DATA_PORT, (divisor >> 8) & 0xFF);

	// Enable IRQ0 in the PIC
    ioport_out(PIC1_DATA_PORT, ioport_in(0x21) & ~(1 << 0));
}

// purpose: handles interrupt raised by PIT
void handle_clock_interrupt() {
	// clear interrupt; tells PIC we
	// are handling it.
	ioport_out(PIC1_COMMAND_PORT, 0x20);

	switch_process_from_queue();
}

// purpose: creates an IDT entry 
// interrupt_num: index in the IDT
// offset: address of interrupt handler
// flags: IDT flags (see https://wiki.osdev.org/Interrupt_Descriptor_Table)
void idt_set_descriptor(uint8_t interrupt_num, void* offset, uint8_t flags) {
	IDT[interrupt_num].offset_lowerbits = (uint32_t)offset & 0x0000FFFF;
	IDT[interrupt_num].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[interrupt_num].zero = 0;
	IDT[interrupt_num].type_attr = flags;
	IDT[interrupt_num].offset_upperbits = ((uint32_t)offset & 0xFFFF0000) >> 16;
}

// purpose: sets up IDT, installs interrupt handlers, initializes interrupt
// 			controllers (PICs), and finally informs the CPU of the IDT.
// much thanks to PageKey on YouTube for the tutorial. 
// https://www.youtube.com/watch?v=YtnNX074jMU
void init_idt() {
	for (int i = 0; i < EXCEPTIONS_SIZE; i++) {
		idt_set_descriptor(i, isr_stub_table[i], IDT_TRAP_GATE_32BIT);
	}

	idt_set_descriptor(0x20, (void*)clock_handler,     IDT_TRAP_GATE_32BIT);
	idt_set_descriptor(0x21, (void*)keyboard_handler,  IDT_INTERRUPT_GATE_32BIT);
	idt_set_descriptor(0x80, (void*)syscall_handler,   IDT_INTERRUPT_GATE_32BIT);

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

// purpose: during multi-processing, this "process" provides ensures there is
//			always a process to switch from/to when all other processes exit.
//			it will not be scheduled unless no other processes are active and 
//			thus should not come up in the course of normal use. 
// NOTE: currently this is a while(1); (which is all it ever should be) but 
//       with extra, useless steps. since terminal_main must have exited before
//       this ever runs, its output will never reach the screen.
void kernel_backstop() {
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


// purpose: stand up intial systems of shompOS and initialize base processes.
//			once interrupts are enabled, the PIT will begin firing and control
//			will never return here, rather juggled between any active processes
void kernel_main() {
  	init_heap(HEAP_LOWER_BOUND);
  	init_idt();
  	init_kb();
    ramfs_init_fd_system();
    ramfs_dir_t* root = system_root = init_fs();
    current_dir = root;
    init_stdio(root);
    if (!current_dir) {
        char* str = "Failed to initialize filesystem.";
		ramfs_write(STDOUT_FILENO, str, strlen(str));
        return;
    }

	// Start necessary processes
    init_process(&kernel_backstop, allocate(500));
    init_process(&terminal_main, allocate(500));

    // Initial terminal prompt
	char* str = "ShompOS initialized successfully!\n" 
				"Type 'help' for available commands\n"
				"shompOS> ";
	ramfs_write(STDOUT_FILENO, str, strlen(str));

	// Start sample processes
	// init_process(&sample_text, allocate(500));
    init_process(&sample_count, allocate(500));
    // init_process(&sample_color, allocate(500));
    // init_process(&sample_return, allocate(500));

    init_pit(PIT_DIVISOR);
    enable_interrupts();

    // Main kernel loop. *SHOULD* never actually execute.
    while(1);
}

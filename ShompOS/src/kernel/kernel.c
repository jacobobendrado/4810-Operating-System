// kernel.c
// Core functionality for the kernel
// Cedarville University 2024-25 OSDev Team
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

// ----- experimental attempt to run commands
#define CMD_MAX_LEN 64

// ----- Includes -----
#include <kernel/kernel.h>
#include <kernel/boot.h>
#include <fake_libc/fake_libc.h> // Is this still relevant?
#include <memory/heap.h>
#include <IO/keyboard_map.h>
#include <IO/keyboard_map_shift.h>
#include <string.h>
#include <ramfs.h>
#include <ramfs_executables.h>


// ----- Global variables -----
// This is our entire IDT. Room for 256 interrupts
IDT_entry IDT[IDT_SIZE];
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
KEY_state special_key_state = {0,0,0,0,0};
uint8_t control_key_flags = 0;

// ----- debugging/example variables -----
bool memory_mode = false;
bool input_mode = false;
uint32_t input_len = 0;
char* input_ptr = NULL;
uint8_t alloc_size = 0;
void* ptr[10];

// ----- experimental attempt to run commands
#define CMD_MAX_LEN 64
char cmd_buffer[CMD_MAX_LEN];
size_t cmd_pos = 0;
ramfs_dir_t* current_dir = NULL;
ramfs_dir_t* system_root = NULL;

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

void terminal_writeint(int num) {
    char buffer[12];  // Buffer for storing the integer as a string (including negative sign and null terminator)
    int i = 0;

    // Handle the special case when the number is 0
    if (num == 0) {
        terminal_putchar('0');
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        terminal_putchar('-');
        num = -num;  // Make the number positive for further processing
    }

    // Convert integer to string in reverse order
    while (num > 0) {
        buffer[i++] = (num % 10) + '0';  // Get the last digit and convert it to character
        num /= 10;  // Remove the last digit
    }

    // Print the digits in reverse order
    for (--i; i >= 0; i--) {
        terminal_putchar(buffer[i]);  // Print each digit from the buffer
    }
}


void terminal_writestring(const char* data)
{
	terminal_write(data, strlen(data));
}
void kill_process() {
	__asm__ volatile ("cli; hlt");
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

void exception_handler() {
	terminal_writestring("Exception!");
	kill_process();
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

	for (int i = 0; i < EXCEPTIONS_SIZE; i++) {
		idt_set_descriptor(i, isr_stub_table[i], IDT_TRAP_GATE_32BIT);
	}

	offset = (unsigned int)syscall_handler;
	idt_set_descriptor(0x80, (void*)offset,  IDT_INTERRUPT_GATE_32BIT);

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
    ioport_out(PIC1_COMMAND_PORT, 0x20);
    unsigned char status = ioport_in(KEYBOARD_STATUS_PORT);
    if (status & 0x1) {
        char keycode = ioport_in(KEYBOARD_DATA_PORT);
        // Handle special keys
        if((uint8_t)keycode == 0xE0 || (uint8_t)keycode == 224) {
            keycode = ioport_in(KEYBOARD_DATA_PORT);
        }
        // Handle modifier keys
        if (keycode == 0x2A || (uint8_t)keycode == 0xAA ||
            keycode == 0x36 || (uint8_t)keycode == 0xB6) {
            special_key_state.shift = 1 - ((uint8_t)keycode >> 7);
            return;
        }
        else if (keycode == 0x38 || (uint8_t)keycode == 0xB8) {
            special_key_state.alt = 1 - ((uint8_t)keycode >> 7);
            return;
        }
        else if (keycode == 0x1D || (uint8_t)keycode == 0x9D) {
            special_key_state.ctrl = 1 - ((uint8_t)keycode >> 7);
            return;
        }
        else if(keycode == 0x3A) {
            special_key_state.caps = 1 - special_key_state.caps;
            return;
        }
        // Ignore key releases
        if ((uint8_t)keycode > 127) return;
        // Handle enter key - process command
        if (keyboard_map[(uint8_t)keycode] == '\n') {
            cmd_buffer[cmd_pos] = '\0';
            ramfs_write(STDIN_FILENO, '\n', 1);
            ramfs_write(STDOUT_FILENO, '\n', 1); // Not working
            terminal_putchar('\n');
            if (current_dir && cmd_pos > 0) {
                handle_command(cmd_buffer);
            }
            cmd_pos = 0;
            terminal_writestring("shompOS> ");
            return;
        }
        // Handle backspace (scan code 0x0E)
        else if (keycode == 0x0E) {
            if (cmd_pos > 0) {
                cmd_pos--;
                if (terminal_column > strlen("shompOS> ")) {
                    terminal_column--;
                    terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
                    terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(' ', terminal_color);
                }
            }
            return;
        }
        // Add character to command buffer
        else if (cmd_pos < CMD_MAX_LEN - 1) {
            char c = keyboard_map[(uint8_t)keycode];
            if (c >= 'a' && c <= 'z') {
                if ((special_key_state.shift ^ special_key_state.caps) == 1) {
                    c -= 32;
                }
            } else if (special_key_state.shift) {
                c = keyboard_map_shift[(uint8_t)keycode];
            }
            if (c >= 32 && c <= 126) {  // Only printable characters
                cmd_buffer[cmd_pos++] = c;
                ramfs_write(STDIN_FILENO, &c, 1);
                ramfs_write(STDOUT_FILENO, &c, 1);
            }
        }
    }
}

// Modified command handling in handle_keyboard_interrupt in kernel.c
void parse_command(char* cmd_buffer, char* argv[], int* argc) {
    *argc = 0;
    char* token = cmd_buffer;
    bool in_quotes = false;
    bool was_space = true;  // Track if previous char was space

    for (int i = 0; cmd_buffer[i] != '\0'; i++) {
        if (cmd_buffer[i] == '"') {
            in_quotes = !in_quotes;
            cmd_buffer[i] = '\0';  // Replace quote with null terminator
            if (!in_quotes && token != &cmd_buffer[i]) {
                argv[*argc] = token;
                (*argc)++;
            }
            token = &cmd_buffer[i + 1];
        }
        else if (cmd_buffer[i] == ' ' && !in_quotes) {
            cmd_buffer[i] = '\0';
            if (!was_space) {
                argv[*argc] = token;
                (*argc)++;
            }
            token = &cmd_buffer[i + 1];
            was_space = true;
        }
        else {
            if (was_space) {
                token = &cmd_buffer[i];
                was_space = false;
            }
        }
    }

    // Add the last token if it exists
    if (!was_space && token != &cmd_buffer[strlen(cmd_buffer)]) {
        argv[*argc] = token;
        (*argc)++;
    }
}

void execute_command(char* cmd_buffer) {
    char* argv[16];  // Maximum 16 arguments
    int argc = 0;

    parse_command(cmd_buffer, argv, &argc);

    if (argc == 0) return;  // Empty command

    if (strcmp(argv[0], "clear") == 0) {
        terminal_clear();
    }
    else if (strcmp(argv[0], "ls") == 0) {
        ramfs_ls(current_dir);
    }
    else if (strcmp(argv[0], "pwd") == 0) {
        ramfs_pwd(current_dir);
    }
    else if (strcmp(argv[0], "cat") == 0) {
        if (argc < 2) {
            terminal_writestring("Usage: cat <filename>\n");
        } else {
            ramfs_cat(current_dir, argv[1]);
        }
    }
    else if (strcmp(argv[0], "touch") == 0) {
        if (argc < 2) {
            terminal_writestring("Usage: touch <filename>\n");
        } else {
            ramfs_touch(current_dir, argv[1]);
        }
    }
    else if (strcmp(argv[0], "mkdir") == 0) {
        if (argc < 2) {
            terminal_writestring("Usage: mkdir <dirname>\n");
        } else {
            ramfs_mkdir(current_dir, argv[1]);
        }
    }
    else if (strcmp(argv[0], "rm") == 0) {
        if (argc < 2) {
            terminal_writestring("Usage: rm <filename>\n");
        } else {
            ramfs_rm(current_dir, argv[1]);
        }
    }
    else if (strcmp(argv[0], "help") == 0) {
        terminal_writestring("Available commands:\n");
        terminal_writestring("  clear - Clear the screen\n");
        terminal_writestring("  ls - List directory contents\n");
        terminal_writestring("  pwd - Print working directory\n");
        terminal_writestring("  cat <file> - Display file contents\n");
        terminal_writestring("  touch <file> - Create empty file\n");
        terminal_writestring("  mkdir <dir> - Create directory\n");
        terminal_writestring("  rm <file> - Remove file\n");
    }
    else {
            terminal_writestring("Unknown command: ");
            terminal_writestring(argv[0]);
            terminal_writestring("\nType 'help' for available commands\n");
        }
    }

void handle_command(char* cmd) {
    // Split command and arguments
    char* cmd_name = cmd;
    char* args = NULL;

    // Find first space to separate command from args
    for (size_t i = 0; cmd[i] != '\0'; i++) {
        if (cmd[i] == ' ') {
            cmd[i] = '\0';  // Split string
            args = &cmd[i + 1];
            break;
        }
    }

    if (strcmp(cmd_name, "clear") == 0) {
        terminal_clear();
    }
    else if (strcmp(cmd_name, "ls") == 0) {
        ramfs_ls(current_dir);
    }
    else if (strcmp(cmd_name, "pwd") == 0) {
        ramfs_pwd(current_dir);
    }
    else if (strcmp(cmd_name, "cat") == 0) {
        if (!args) {
            terminal_writestring("Usage: cat <filename>\n");
            return;
        }
        ramfs_cat(current_dir, args);
    }
    else if (strcmp(cmd_name, "touch") == 0) {
        if (!args) {
            terminal_writestring("Usage: touch <filename>\n");
            return;
        }
        ramfs_touch(current_dir, args);
    }
    else if (strcmp(cmd_name, "mkdir") == 0) {
        if (!args) {
            terminal_writestring("Usage: mkdir <dirname>\n");
            return;
        }
        ramfs_mkdir(current_dir, args);
    }
    else if (strcmp(cmd_name, "rm") == 0) {
        if (!args) {
            terminal_writestring("Usage: rm <filename>\n");
            return;
        }
        ramfs_rm(current_dir, args);
    }
    else if (strcmp(cmd_name, "help") == 0) {
        terminal_writestring("Available commands:\n");
        terminal_writestring("  clear        Clear the screen\n");
        terminal_writestring("  ls          List directory contents\n");
        terminal_writestring("  pwd         Print working directory\n");
        terminal_writestring("  cat <file>  Display file contents\n");
        terminal_writestring("  touch <file> Create empty file\n");
        terminal_writestring("  mkdir <dir> Create directory\n");
        terminal_writestring("  rm <file>   Remove file\n");
        terminal_writestring("  help        Show this help message\n");
    }
    else if (strcmp(cmd_name, "cd") == 0) {
        if (!args) {
            terminal_writestring("Usage: cd <filename>\n");
            return;
        }
        ramfs_dir_t *result_dir = ramfs_cd(system_root, args);
        if (result_dir) {
            current_dir = result_dir;
        }
        else {
            terminal_writestring("Failed to find directory\n");
        }

    }
    else if (cmd_name[0] != '\0') {
        terminal_writestring("Unknown command: ");
        terminal_writestring(cmd_name);
        terminal_writestring("\n");
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
    ramfs_init_fd_system();
    ramfs_dir_t* root = system_root = ramfs_create_root();
    current_dir = root;
    init_stdio(root);

	while(1);
}

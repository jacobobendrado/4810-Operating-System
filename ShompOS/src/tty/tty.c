#include <tty.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kernel.h>
#include <kernel/boot.h>
#include <string.h>
#include <ramfs.h>
#include <ramfs_executables.h>
#include <keyboard_map.h>
#include <keyboard_map_shift.h>

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
ramfs_dir_t* current_dir = NULL;
ramfs_dir_t* system_root = NULL;

// ----- experimental attempt to run commands
#define CMD_MAX_LEN 64
char cmd_buffer[CMD_MAX_LEN];
size_t cmd_pos = 0;

// --- input ---
KEY_state special_key_state = {0,0,0,0,0};
uint8_t control_key_flags = 0;

void terminal_writestring(const char* data);

// ----- Bare Bones -----
void init_kb()
{
	ioport_out(PIC1_DATA_PORT, 0xFD);
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
	// char* term = "shompOS>";
	// terminal_writestring(term);
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


void terminal_clear() {
	for (uint8_t y = 0; y < VGA_HEIGHT; y++){
		for (uint8_t x = 0; x < VGA_WIDTH; x++){

			terminal_putentryat(' ', terminal_color, x, y);
		}
	}
	terminal_column = 0;
	terminal_row = 0;
}

void terminal_refresh() {
	for (uint8_t y = 0; y < VGA_HEIGHT; y++){
		for (uint8_t x = 0; x < VGA_WIDTH; x++){
			size_t index = y * VGA_WIDTH + x;
			uint16_t oc = terminal_buffer[index];
			terminal_putentryat(oc, terminal_color, x, y);
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

     terminal_writestring("\n");

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
             terminal_writestring("Usage: cd b<directory>\n");
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
     else if (strcmp(cmd_name, "run") == 0) {
         if (!args) {
             terminal_writestring("Usage: rm <filename>\n");
             return;
         }
         ramfs_run(current_dir, args);

     }
     else if (cmd_name[0] != '\0') {
         terminal_writestring("Unknown command: ");
         terminal_writestring(cmd_name);
         terminal_writestring("\n");
     }
}


void handle_keyboard_interrupt() {
    ioport_out(PIC1_COMMAND_PORT, 0x20);
    unsigned char status = ioport_in(KEYBOARD_STATUS_PORT);
    if (status & 0x1) {
        char keycode = ioport_in(KEYBOARD_DATA_PORT);
        // Handle special keys
        if((uint8_t)keycode == 0xE0) {
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
                // ramfs_write(STDIN_FILENO, &c, 1);
                ramfs_write(STDOUT_FILENO, &c, 1);
            }
        }
    }
}

void cowsay(const char *message) {
    // Calculate length of message manually
    int len = 0;
    while (message[len] != '\0') {
        len++;
    }

    // Top of speech bubble
    terminal_writestring(" ");
    for (int i = 0; i < len + 2; i++) terminal_writestring("_");
    terminal_writestring("\n");

    // Message line
    terminal_writestring("< ");
    terminal_writestring(message);
    terminal_writestring(" >\n");

    // Bottom of speech bubble
    terminal_writestring(" ");
    for (int i = 0; i < len + 2; i++) terminal_writestring("-");
    terminal_writestring("\n");

    // Cow
    terminal_writestring("        \\   ^__^\n");
    terminal_writestring("         \\  (oo)\\_______\n");
    terminal_writestring("            (__)\\       )\\/\\\n");
    terminal_writestring("                ||----w |\n");
    terminal_writestring("                ||     ||\n");
}

// char* str = "";
// ramfs_write(STDOUT_FILENO, str, strlen(str));
// Constantly checking stdin, and writing to screen
void terminal_main() {
    cowsay("Mooo. Welcome to ShompOS!");
    while(1) {
        // read up to one full line from stdin
        char printBuf[81] = {0};
        if (ramfs_read(STDIN_FILENO, printBuf, 80)) {
            terminal_writestring(printBuf);
        }
    }
}

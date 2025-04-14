// tty.c
// Screen management
// Cedarville University 2024-25 OSDev Team

#include <tty.h>
#include <kernel.h>
#include <boot.h>
#include <string.h>
#include <keyboard_map.h>
#include <keyboard_map_shift.h>


// IO Ports for Keyboard
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
// Screen Sizes
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// ----- experimental attempt to run commands -----
#define CMD_MAX_LEN 64
char cmd_buffer[CMD_MAX_LEN];
size_t cmd_pos = 0;

// ------ Output -----
// the output of shompOS is a 80x25 grid of characters, represented in memory 
// by vga entries— character/color byte pairs. the region 0xB8000...0xBF9F 
// stores the 2000 entries that will be displayed on screen.
uint16_t* terminal_buffer;
entry_color terminal_color;
// state variables to track the cursor's location 
size_t terminal_row;
size_t terminal_column;

// ----- File System ----
ramfs_dir_t* current_dir = NULL;
ramfs_dir_t* system_root = NULL;

// --- Input ---
KEY_state special_key_state = {0,0,0,0,0};


// ----- Terminal -----
// purpose: set up initial terminal state. clears all memory in range
//          0xB8000..0xBF9F to prep for VGA output.
void init_terminal() {
	terminal_buffer = (uint16_t*) 0xB8000;
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_clear();
}

// purpose: advance cursor one row, clearing the screen if necessary
void terminal_advance_row() {
	if (++terminal_row == VGA_HEIGHT){
		terminal_row = 0;
		terminal_clear();
	}

	terminal_column = 0;
}

// purpose: places a character at X,Y screen coordinates in specified color
// c: character to output
// color: entry color created by vga_color_entry()
// x/y: screen coordinates on 80x25 character output
void terminal_putentryat(char c, entry_color color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

// purpose: places a character at the terminal cursor and advances it one space.
// c: character to output
void terminal_putchar(char c) {
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

// purpose: writes a character buffer to the screen starting from the cursor
// data: the first character in buffer
// size: length of buffer
void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

// purpose: writes a null terminated character buffer to the screen, starting 
//          at the cursor
// data: the first character in buffer
void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

// purpose: clears the screen of all characters
void terminal_clear() {
	for (uint8_t y = 0; y < VGA_HEIGHT; y++){
		for (uint8_t x = 0; x < VGA_WIDTH; x++){
			terminal_putentryat(' ', terminal_color, x, y);
		}
	}
	terminal_column = 0;
	terminal_row = 0;
}


// ----- Simple Command Handling -----
// purpose: executes a ramfs command after the user presses ENTER
// cmd: a pointer to the input string
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
        terminal_writestring("  clear          Clear the screen\n");
        terminal_writestring("  ls             List directory contents\n");
        terminal_writestring("  pwd            Print working directory\n");
        terminal_writestring("  cat <file>     Display file contents\n");
        terminal_writestring("  touch <file>   Create empty file\n");
        terminal_writestring("  mkdir <dir>    Create directory\n");
        terminal_writestring("  rm <file>      Remove file\n");
        terminal_writestring("  help           Show this help message\n");
        terminal_writestring("  cd <directory> Change directory\n");
        terminal_writestring("  run <file>     Run an executable\n");
    }
    else if (strcmp(cmd_name, "cd") == 0) {
        if (!args) {
            terminal_writestring("Usage: cd </absolute/path/to/directory>\n");
            return;
        }
        ramfs_dir_t *result_dir = ramfs_find_dir(system_root, args);
        if (result_dir) {
            current_dir = result_dir;
        }
        else {
            terminal_writestring("Failed to find directory\n");
        }

    }
    else if (strcmp(cmd_name, "run") == 0) {
        if (!args) {
            terminal_writestring("Usage: run <filename>\n");
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


// ----- Keyboard Handling -----
void init_kb() {
	ioport_out(PIC1_DATA_PORT, 0xFD);
}

// purpose: reads a keycode from the keyboard and handles it appropriately.
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
            } else {
                ramfs_write(STDOUT_FILENO, "\n", 1);
            }
            cmd_pos = 0;
            char* str = "shompOS> ";
            ramfs_write(STDOUT_FILENO, str, strlen(str));
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
                ramfs_write(STDOUT_FILENO, &c, 1);
            }
        }
    }
}


// ----- Sample Processes -----
void sample_text() {
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

void sample_count() {
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

void sample_color() {
	uint8_t color = 0;
	uint8_t row = VGA_HEIGHT;

	while (1) {
    	char buf[8] = "       ";

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

void sample_return() {
	char* str = "before wait\n";
	ramfs_write(STDOUT_FILENO, str, strlen(str));
    for (uint32_t i = 0x00FFFFFF; i > 0; i-- ){
        uint8_t color = i;
        terminal_putentryat('X', color, i%5, terminal_row);
    }
	str = "\nafter wait. returning...!";
	ramfs_write(STDOUT_FILENO, str, strlen(str));
}

void cowsay(const char *message) {
    // Calculate length of message manually
    int len = 0;
    while (message[len] != '\0') {
        len++;
    }

    // Top of speech bubble
    terminal_writestring("\xDA");
    for (int i = 0; i < len + 2; i++) terminal_writestring("-");
    terminal_writestring("\xBF\n");

    // Message line
    terminal_writestring("| ");
    terminal_writestring(message);
    terminal_writestring(" |\n");

    // Bottom of speech bubble
    terminal_writestring("\xC0");
    for (int i = 0; i < len + 2; i++) 
        terminal_writestring(i < 9 && i > 5 ? " " : 
                             i == 5 ? "\xBF" :
                             i == 9 ? "\xDA" : "-");
    terminal_writestring("\xD9\n");

    // Cow
    terminal_writestring("       \\  | ^__^\n");
    terminal_writestring("         \\| (oo)\\_______\n");
    terminal_writestring("            (__)\\       )\\/\\\n");
    terminal_writestring("                ||----w |\n");
    terminal_writestring("                ||     ||\n");
}


// ----- Main ----
// purpose: constantly polls STDIN to see if any bytes need to be written to 
//          screen.
void terminal_main() {
    init_terminal();
    // cowsay("Mooo. Welcome to ShompOS!");
    while(1) {
        // read up to one full line from stdin
        char printBuf[81] = {0};
        if (ramfs_read(STDIN_FILENO, printBuf, 80)) {
            terminal_writestring(printBuf);
        }
    }
}

// Use the following to write to screen
// char* str = "";
// ramfs_write(STDOUT_FILENO, str, strlen(str));
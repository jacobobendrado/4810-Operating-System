#include <stdarg.h>  // For variable arguments

// Simple putchar
void putchar(char c) {
    // Write the character to some output, such as VGA memory or serial port
    // Assuming a function "write_char_to_console" exists (you need to implement this part)
    write_char_to_console(c);  
}

// Simple puts
void puts(const char *str) {
    while (*str) {
        putchar(*str++);
    }
    putchar('\n');
}


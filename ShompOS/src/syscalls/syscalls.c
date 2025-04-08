// syscalls.c
// Implementation of syscall functions
// Cedarville University 2024-25 OSDev Team

#include <kernel.h>
#include <tty.h>

void syscall_exit(int error_code) {
    terminal_writestring("exiting!");
    char myString[2] = { (char)error_code + '0', '\0' };
    terminal_writestring(myString);
    return;
}
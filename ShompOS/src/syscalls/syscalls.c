// syscalls.c
// Implementation of syscall functions
// Cedarville University 2024-25 OSDev Team

#include <kernel.h>
#include <string.h>

void syscall_exit(int error_code) {
    char* str = "exiting with code ";
    char myString[2] = { (char)error_code + '0', '\0' };
    ramfs_write(STDOUT_FILENO, str, strlen(str));
    ramfs_write(STDOUT_FILENO, myString, strlen(myString));
    return;
}
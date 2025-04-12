// syscalls.c
// Implementation of syscall functions
// Cedarville University 2024-25 OSDev Team

#include <kernel.h>
#include <string.h>
#include <process.h>

void syscall_exit(int error_code) {
    if (error_code >= 0 && error_code < 10) {
        char* str = "exiting with code ";
        char myString[2] = { (char)error_code + '0', '\0' };
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        ramfs_write(STDOUT_FILENO, myString, strlen(myString));
    } else {
        char* str = "exiting";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
    }
    kill_process(active_pid);
}
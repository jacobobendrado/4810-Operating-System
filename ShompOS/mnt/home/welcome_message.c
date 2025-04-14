// welcome_message.c
// Test write() syscall
// Cedarville University 2024-25 OSDev Team

#include "../inc/syscalls.h"
#define STDOUT 1

int main() {
	write(STDOUT, "Hello world!", 13);

	return 0;
}

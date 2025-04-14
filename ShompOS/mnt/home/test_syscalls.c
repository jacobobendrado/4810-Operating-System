// test_syscalls.c
// Test exit() syscall
// Cedarville University 2024-25 OSDev Team

#include "../inc/syscalls.h"

int main() {
	exit(2);
	write(1, "after", 6); // Should not be reached

	return 0;
}

// fake_libc.c
// Library functions for kernel
// Cedarville University 2024-25 OSDev Team

#include <stddef.h>
#include <stdint.h>
#include <fake_libc.h>

inline int max(int a, int b) {
	return a >= b ? a : b;
}

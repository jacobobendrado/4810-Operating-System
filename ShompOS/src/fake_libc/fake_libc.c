#include <stddef.h>
#include <stdint.h>
#include <fake_libc.h>


inline int max(int a, int b) {
	return a >= b ? a : b;
}

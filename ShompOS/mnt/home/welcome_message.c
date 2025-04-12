
#include "../inc/syscalls.h"
#define STDOUT 1

int main() {
	write(STDOUT, "Hello world!", 13);

	return 0;
}

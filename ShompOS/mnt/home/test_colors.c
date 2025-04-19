#include "../inc/syscalls.h"
#define STDOUT 1

void main() {
	uint8_t color = 0;
	uint8_t row = 25;

	while (1) {
    	char buf[8] = "       ";

		for (uint8_t i = 7; i > 0 ; i--){
			putchar(buf[i], color, 68+i, row);
		}
		if (--row == (uint8_t)-1) {
			row = 25;
			color += 0x10;
		}
		for (uint32_t i = 0xFFFFFF; i > 0; i-- );
	}
}
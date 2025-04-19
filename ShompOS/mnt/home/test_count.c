#include "../inc/syscalls.h"
#define STDOUT 1

void addr_to_string(char* buffer, uintptr_t addr) {
    char hex_digits[] = "0123456789ABCDEF";
    buffer[0] = '0';
    buffer[1] = 'x';
    
    // Handle 0 specially
    if (addr == 0) {
        buffer[2] = '0';
        buffer[3] = '\0';
        return;
    }
    
    // Find first significant digit position
    int digits = 0;
    uintptr_t temp = addr;
    while (temp) {
        digits++;
        temp >>= 4;
    }
    if (digits == 1) digits++;
    
    // Write digits from most to least significant
    buffer[digits + 2] = '\0';  // +2 for "0x" prefix
    int pos = digits + 1;       // Position to write next digit
    
    while (addr) {
        buffer[pos--] = hex_digits[addr & 0xF];
        addr >>= 4;
    }
}

void main() {
    const uint8_t buf_len = 4;
	uint8_t color = 0;
	uint8_t row = 0;

	while (1) {
    	char buf[buf_len+1];
    	addr_to_string(buf, (uintptr_t)color);

        for (uint8_t i = 4; i > 0 ; i--){
            putchar(buf[4-i], color, 80-i, row);
		}
		// write(STDOUT, buf, buf_len);
		color++;
		if (++row >= 25) row = 0;
		for (uint32_t i = 0xFFFFFFF; i > 0; i-- );
	}
}
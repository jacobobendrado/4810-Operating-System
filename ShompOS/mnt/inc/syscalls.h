

#include <stdint.h>


void exit(int32_t error_code);
uint32_t read(uint32_t fd, char *buf, uint32_t count);
uint32_t write(uint32_t fd, const char *buf, uint32_t count);
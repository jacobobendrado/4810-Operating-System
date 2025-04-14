// mnt/inc/syscalls.h
// Header for library to wrap syscalls
// Cedarville University 2024-25 OSDev Team

#include <stdint.h>


void exit(int32_t error_code);
uint32_t read(uint32_t fd, char *buf, uint32_t count);
uint32_t write(uint32_t fd, const char *buf, uint32_t count);
uint32_t open(const char *filename, int flags, uint32_t mode);
uint32_t close(uint32_t fd);
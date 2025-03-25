
#include "syscalls.h"

extern uint32_t do_syscall(const uint32_t eax, const uint32_t ebx, const uint32_t ecx, const uint32_t edx, const uint32_t esi, const uint32_t edi, const uint32_t ebp);

void exit(int32_t error_code) {
    do_syscall(1, error_code, 0, 0, 0, 0, 0);
}

uint32_t read(uint32_t fd, char *buf, uint32_t count) {
    do_syscall(3, fd, buf, count, 0, 0, 0);
}

uint32_t write(uint32_t fd, const char *buf, uint32_t count) {
    do_syscall(4, fd, buf, count, 0, 0, 0);
}
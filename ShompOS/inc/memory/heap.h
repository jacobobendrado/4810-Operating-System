#pragma once

#include <stddef.h>
#include <stdint.h>
#include <fake_libc/fake_libc.h>

#define HEAP_LOWER_BOUND 0x4000000
#define MAX_BLOCK_SCALE 0x0F
#define MIN_BLOCK_SCALE 0x04

// sits at the 9 LSB of every memory block. a block's size in bytes is
// 2^block_header.scale
// the minimum allocation is 16 bytes to acommodate the 9 bytes header.
// the max allocation size is UNREASONABLY large and would not see use
// on most (any) systems.
typedef struct _block_header {
    list_header list;
    uint8_t is_free : 1;
    uint8_t scale : 7;
} __attribute__((packed)) block_header;

void init_heap();
void* allocate(size_t request_size);
void free(void** block_ptr);
int8_t brk(void* addr);
int8_t sbrk(int32_t inc);

// ----- FOR DEBUGGING ------
void print_free_counts();
char* addr_to_string(char* buffer, uintptr_t addr);
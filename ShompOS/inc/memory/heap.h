// heap.c
// Memory management
// Cedarville University 2024-25 OSDev Team

#pragma once

#include <stddef.h>
#include <stdint.h>

#define HEAP_LOWER_BOUND 0x4000000
#define MAX_BLOCK_SCALE 0x0F // This is able to be changed
#define MIN_BLOCK_SCALE 0x04 // This should not

// generic struct for any linked lists. 8 bytes.
// may be best to move to another location, currently is only used here.
typedef struct _list_header {
    struct _list_header* next;
    struct _list_header* prev;
} list_header;

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
uint8_t free(void* data);
int8_t brk(void* addr);
int8_t sbrk(int32_t inc);

// ----- FOR DEBUGGING ------
// void print_free_counts();
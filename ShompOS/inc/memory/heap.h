#pragma once
#include <stddef.h>
#include <stdint.h>
#include <kernel/kernel.h>


#define HEAP_LOWER_BOUND 0x4000000
#define HEAP_UPPER_BOUND 0x5000000
#define DEFAULT_HEAP_SCALE 0x19
#define MAX_BLOCK_SCALE 0x7F
#define MIN_BLOCK_SCALE 0x2

// generic struct for any linked lists. 8 bytes.
typedef struct _list_header {
    struct _list_header* next;
    struct _list_header* prev;
} list_header;

// sits at the 9 LSB of every memory block. 
// a block's size in bytes is 2^block_header.scale
// the minimum allocation is 16 bytes to acommodate the 9 bytes header.
// the max allocation size is UNREASONABLY large and would not see use
// on most (any) systems.
typedef struct _block_header {
    list_header list;
    uint8_t is_free : 1;
    uint8_t scale : 7;
}  block_header;

void init_heap();
void* simple_allocate(size_t size);


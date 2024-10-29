#include <memory/heap.h>
#include <kernel/kernel.h>

list_header free_list[MAX_BLOCK_SCALE];
list_header used_list[MAX_BLOCK_SCALE];

void init_heap() {
    // create two tables of list_headers. which represent
    // free and used blocks of memory for each of the 128 
    // valid scales. 
    for (uint8_t i = 0; i < MAX_BLOCK_SCALE-1; i++) {
        free_list[i].prev = &free_list[i];
        free_list[i].next = &free_list[i];

        used_list[i].prev = &used_list[i];
        used_list[i].next = &used_list[i];
    }

    // create default heap from 0x4000000..0x5000000
    // this block is scale 28 (2^25 bytes) 
    // the scale and address are copied from MentOS but also changed
    // slightly. we may want our heap in another location.
    block_header* full_heap = (block_header*)HEAP_LOWER_BOUND;
    free_list[DEFAULT_HEAP_SCALE].next = (list_header*)full_heap;
    
    full_heap->list.next = (list_header*)full_heap;
    full_heap->list.prev = &free_list[DEFAULT_HEAP_SCALE];
    full_heap->is_free = 1;
    full_heap->scale = DEFAULT_HEAP_SCALE;
}

block_header* __blkmngr_find_fit(size_t request_scale) {
    // the list entry is the first field in a block_header,
    // a cast can access the block_header fields as a "super-set"
    // of a list_header
    block_header* curr = (block_header*)&free_list[request_scale];
    block_header* best_fit = NULL;

    // TODO: make scan larger scales as well
    // self-looping next indicates end-of-list
    // NULL next indicates error
    while ((block_header*)curr->list.next != curr && curr->list.next != NULL){
        curr = (block_header*)(curr->list.next);
        if (request_scale <= curr->scale && (best_fit == NULL || (curr->scale < best_fit->scale))) {
            // update best_fit and break if its perfect
            best_fit = curr; 
            if (request_scale == best_fit->scale) break;
        }
    } 
    return best_fit;
}

// TODO: __blkmngr_split_block()

// TODO: __blkmngr_coalesce_block() {
//      find buddy using size and addr. 
// }

// TODO: simple_free()

void* simple_allocate(size_t raw_size) {
    // reject invalid reqests
    if (!raw_size) return NULL;

    uint32_t request_scale = raw_size; // TODO: get scale not size

    block_header* block = __blkmngr_find_fit(request_scale);

    if (block) {
        char* str = "\nallocation success\n";
        terminal_writestring(str);

        if (request_scale < block->scale) {
            // __blkmngr_split_block()
            char* str = "\ntoo big of block. splitting...\n";
            terminal_writestring(str);
        }

    } else {
        char* str = "\nallocation failed\n";
        terminal_writestring(str);
        return NULL;
    }
    return block;
}



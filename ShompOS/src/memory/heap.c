#include <stdbool.h>
#include <memory/heap.h>
#include <kernel/kernel.h>
#include <fake_libc/list_header.h>

list_header free_list[MAX_BLOCK_SCALE];
list_header used_list[MAX_BLOCK_SCALE];


void init_heap(void* heap_addr) {
    // create two tables of list_headers. which represent
    // free and used blocks of memory for each of the 128 
    // valid scales. 
    for (uint8_t i = MIN_BLOCK_SCALE; i <= MAX_BLOCK_SCALE; i++) {
        free_list[i].prev = &free_list[i];
        free_list[i].next = &free_list[i];

        used_list[i].prev = &used_list[i];
        used_list[i].next = &used_list[i];
    }

    // create default heap as a single block, scale 28 (2^25 bytes)
    // the scale and address are copied from MentOS but also changed
    // slightly. we may want our heap in another location.
    block_header* full_heap = (block_header*)heap_addr;
    free_list[DEFAULT_HEAP_SCALE].next = (list_header*)full_heap;
    
    full_heap->list.next = (list_header*)full_heap;
    full_heap->list.prev = &free_list[DEFAULT_HEAP_SCALE];
    full_heap->is_free = 1;
    full_heap->scale = DEFAULT_HEAP_SCALE;
}

block_header* __blkmngr_find_fit(uint8_t request_scale) {
    uint8_t search_scale = request_scale;
    // the list_header is the first field in a block_header,
    // a cast can access the block_header fields as a "super-set"
    // of a list_header
    block_header* curr = (block_header*)&free_list[search_scale];
    block_header* best_fit = NULL;

    // find smallest block that will accommodate request
    // return a larger block if necessary
    while (is_end_of_list((list_header*)curr) 
        && search_scale < MAX_BLOCK_SCALE) {
        curr = (block_header*)&free_list[++search_scale];
    }

    // if one is found, update best_fit and remove from free_list
    if (!is_end_of_list((list_header*)curr) && curr->list.next != NULL){
        // update best fit
        curr = (block_header*)(curr->list.next);
        best_fit = curr; 

        // remove from free_list
        if (is_end_of_list((list_header*)curr)) {
            free_list[search_scale].next = &free_list[search_scale];
        } else {
            free_list[search_scale].next = curr->list.next;
        }
        curr->list.next = (list_header*)curr;
        curr->list.prev = (list_header*)curr;
        curr->is_free = 0;
    }

    return best_fit;
}

void __blkmngr_split_block(block_header* block) {
    // reject small and NULL blocks
    if (!block || block->scale <= MIN_BLOCK_SCALE) return;

    // place new block_header 2^scale bytes away 
    block->scale--;
    block_header* new_block = (block_header*)(block + (1<<block->scale));

    // populate new_block header
    new_block->list.prev = &free_list[new_block->scale];
    new_block->is_free = 1;
    new_block->scale = block->scale;
    if (is_end_of_list(&free_list[new_block->scale])) {
        new_block->list.next = (list_header*)new_block;
    } else {
        new_block->list.next = free_list[new_block->scale].next;
    }

    // insert new_block into free_list
    free_list[new_block->scale].next = (list_header*)new_block;
}

// TODO: __blkmngr_coalesce_block() {
//      find buddy using size and addr. 
// }

// TODO: simple_free()

void* simple_allocate(size_t raw_size) {
    // reject invalid reqests
    if (!raw_size) return NULL;

    uint32_t request_scale = max(raw_size, MIN_BLOCK_SCALE); // TODO: get scale not size

    block_header* block = __blkmngr_find_fit(request_scale);

    if (block) {
        while (block->scale > request_scale) {
            __blkmngr_split_block(block);
            char* str = "too big of block. splitting...\n";
            terminal_writestring(str);
        }
        char* str = "allocation success\n";
        terminal_writestring(str);


    } else {
        char* str = "\nallocation failed\n";
        terminal_writestring(str);
        return NULL;
    }
    return block;
}



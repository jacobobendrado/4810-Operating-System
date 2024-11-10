#include <stdbool.h>
#include <memory/heap.h>
#include <kernel/kernel.h>
#include <fake_libc/fake_libc.h>

// an array of FIFO linked lists which represents free memory blocks for
// each of the valid scales. 
list_header free_list[MAX_BLOCK_SCALE+1];

// pupose: constructs the initial state of the heap: a single, contiguous block
// heap_addr: a pointer the LSB of the heap to initialize. 
void init_heap(void* heap_addr) {
    // populate free_list such that each entry points to itself.
    for (uint8_t i = MIN_BLOCK_SCALE; i <= MAX_BLOCK_SCALE; i++) {
        free_list[i].prev = &free_list[i];
        free_list[i].next = &free_list[i];
    }

    // by default, the heap will be a single, massive block.
    block_header* full_heap = (block_header*)heap_addr;
    free_list[DEFAULT_HEAP_SCALE].next = (list_header*)full_heap;
    full_heap->list.next = (list_header*)full_heap;
    full_heap->list.prev = &free_list[DEFAULT_HEAP_SCALE];
    full_heap->is_free = 1;
    full_heap->scale = DEFAULT_HEAP_SCALE;
}

// purpose: converts a request from size in bytes to a power of 2 scale
// uint32_t size: requested size to allocate in bytes
// returns: next power of 2, uint8_t
uint8_t size_to_scale(uint32_t size) {
    // add the size of the required header to the request.
    size += sizeof(block_header);
    // scale = size rounded up to the nearest power of 2.
    // clz is a builtin function of GCC that tells us the count of leading
    // zeros in a register. by subtracting that from 32, its essentially
    // giving us the MSB that is 1... or the next power of 2. 
    return 32 - __builtin_clz((size - 1));
    // +--------------------+
    // |requested   returned|
    // |    bytes   scale   |
    // +--------------------+
    // |     1..7 - 4       |
    // |    8..23 - 5       |
    // |   24..55 - 6       |
    // |  56..119 - 7       |
    // | 120..247 - 8       |
    // | 248..503 - 9       |
    // |504..1015 - A       |
    // +--------------------+
}

// purpose: finds the smalled free memory block that will acommodate request
// request_scale: request size as scale
// returns: best fitting block
block_header* __blkmngr_find_fit(uint8_t request_scale) {
    // we may end up searching larger scales if needed
    uint8_t search_scale = request_scale;
    // the list_header is the first field in a block_header,
    // a cast can access the block_header fields as a "super-set"
    // of a list_header
    block_header* curr = (block_header*)&free_list[search_scale];
    block_header* best_fit = NULL;

    // find smallest block that will accommodate request
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
            curr->list.next->prev = &free_list[search_scale];
        }
        curr->list.next = (list_header*)curr;
        curr->list.prev = (list_header*)curr;
        curr->is_free = 0;
    }

    return best_fit;
}

// purpose: splits an oversized block in half and adds one half to free list
//          as a scale-1 block
// block: a pointer to the block to be split
// returns: the other half as a scale-1 block
void __blkmngr_split_block(block_header* block) {
    // reject small and NULL blocks
    if (!block || block->scale <= MIN_BLOCK_SCALE) return;

    // place new block_header 2^scale bytes away 
    block->scale--;
    block_header* new_block = (block_header*)((uint32_t)block + (1<<block->scale));

    // populate new_block header
    new_block->is_free = 1;
    new_block->scale = block->scale;
    new_block->list.prev = &free_list[new_block->scale];
    if (is_end_of_list(&free_list[new_block->scale])) {
        new_block->list.next = (list_header*)new_block;
    } else {
        new_block->list.next = free_list[new_block->scale].next;
        free_list[new_block->scale].next->prev = (list_header*)&new_block;
    }

    // insert new_block into free_list
    free_list[new_block->scale].next = (list_header*)new_block;
}

// purpose: merges a given block with it's buddy block (if buddy is free)
//          into a new scale+1 block. this process repeats until the entire
//          heap is merged or a buddy is full.
// block: a pointer to the block to potentiall merge
// returns: a pointer to the header of the resulting block (or original block
//            if the merge was unsuccessful)
block_header* __blkmngr_coalesce_block(block_header* block) {
	// find buddy using block address & scale.
    block_header* curr_block = block;
    int32_t block_offset = (uint32_t)curr_block - HEAP_LOWER_BOUND;
    int32_t buddy_offset = (block_offset%(1<<(curr_block->scale+1))) ? \
                            -(1<<curr_block->scale) : (1<<curr_block->scale);
    block_header* buddy = (block_header*)((uint32_t)curr_block + buddy_offset);

    // repeatedly attempt a merge
    while(buddy->is_free && buddy->scale == curr_block->scale \
            && curr_block->scale < MAX_BLOCK_SCALE){
        
        // remove buddy from free list
        if (is_end_of_list((list_header*)buddy)) {
            buddy->list.prev->next = buddy->list.prev;
        } else {
            buddy->list.prev->next = buddy->list.next;
            buddy->list.next->prev = buddy->list.prev;
        }

        // curr_block should point to the earliest header
        if (buddy_offset < 0) {
            curr_block = buddy;
        }  

        // grow block scale and search for new buddy
        curr_block->scale += 1;
        block_offset = (uint32_t)curr_block - HEAP_LOWER_BOUND;
        buddy_offset = (block_offset%(1<<(curr_block->scale+1))) ? \
                        -(1<<curr_block->scale) : (1<<curr_block->scale);
        buddy = (block_header*)((uint32_t)curr_block + buddy_offset);
    }
	return curr_block;
}

// purpose: allocates a section of memory dynamically
// request_size: the amount of memory (in bytes) needed
// returns: a pointer to the first free byte
void* allocate(size_t request_size) {
    // reject invalid reqests
    if (!request_size) return NULL;

    uint32_t request_scale = max(size_to_scale(request_size), MIN_BLOCK_SCALE);
    block_header* block = __blkmngr_find_fit(request_scale);

    if (block) {
        while (block->scale > request_scale) {
            __blkmngr_split_block(block);
        }
    } else {
        return NULL;
    }
    // return the first free byte after the block_header. 
    return block+1;
}

// purpose: returns a previously allocated block of memory back into the
//          available pool and clears the pointer.
// data: a pointer to the first free byte after a block_header. (this should be
//       the pointer returned by allocate().)
void free(void** data){
	// reject empty blocks
    if (!data || !*data) return;
    block_header* block = ((block_header*)*data)-1;
	
	// set free bit coalesce block
	block = __blkmngr_coalesce_block(block);
	block->is_free = 1;

	// add to free list
	if (is_end_of_list(&free_list[block->scale])) {
		block->list.next = (list_header*)block;
    } else {
        free_list[block->scale].next->prev = (list_header*)block;
        block->list.next = free_list[block->scale].next;
    }
    free_list[block->scale].next = (list_header*)block;
    block->list.prev = &free_list[block->scale];
    
    // clear pointer
    *data = NULL;
}


// ----- FOR DEBUGGING ------
void print_free_counts(){
    terminal_writestring("free_block counts:\n");
    for (uint8_t i = MIN_BLOCK_SCALE; i <= MAX_BLOCK_SCALE; i++) {
        uint32_t count = 0;
        list_header* curr = &free_list[i];
        while (curr->next != curr)
        {
            count++;
            curr = curr->next;
        }
        char c[2] = {(char)count+48, '\0'};
        char c2[2] = {i==10 ? 'A' : (char)i+48, '\0'};
        terminal_writestring(c);
        terminal_writestring(" free blocks of scale ");
        terminal_writestring(c2);
        terminal_writestring("\n");  
    } 
}

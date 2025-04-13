#include <heap.h>
#include <stdbool.h>
#include <kernel.h>
#include <fake_libc.h>
#include <string.h>


// an array of FIFO linked lists which represents free memory blocks for
// each of the valid scales. 
// slightly non-optimal in terms of memory usage. entries 0..3 remain unused so
// free_list can be indexed with by scale.
static list_header free_list[MAX_BLOCK_SCALE+1];
// the limit of our heap. adjusted through brk() and sbrk().
// points to the first byte AFTER allocatable space.
static void* current_brk = NULL;


static inline bool is_end_of_list(list_header* node) {
    return node->next == node;
}

static inline bool is_head_of_list(list_header* node) {
    return node->prev == node;
}

// purpose: converts a request from size in bytes to a power of 2 scale
// size: requested size to allocate in bytes
// returns: next power of 2, uint8_t
inline uint8_t size_to_scale(uint32_t size) {
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

// purpose: adds a memory block to the appropriate head of free_list and sets
//          is_free to true
// block: the block_header of the block to free
static void __blkmngr_add_to_free_list(block_header* block) {
    list_header* blk = (list_header*)block;
    list_header* list = &free_list[block->scale];

    if (is_end_of_list(list)) {
        //            ╭> [next] -╮
        //            ╰- [prev] <╯
        // ╭> [next] -╮
        // ╰- [prev] <╯    
        //                 |
        //                 v
        // ╭> [next] --> [next] -╮
        // ╰- [prev] <-- [prev] <╯
        blk->next = blk;
    } else {
        //            ╭> [next] -╮
        //            ╰- [prev] <╯
        // ╭> [next] -------------> [next] -- ...
        // ╰- [prev] <------------- [prev] <- ...
        //                 |
        //                 v
        // ╭> [next] --> [next] --> [next] -- ...
        // ╰- [prev] <-- [prev] <-- [prev] <- ...
        blk->next = list->next;
        list->next->prev = blk;
    }

    list->next = blk;
    blk->prev = list;
    block->is_free = 1;
}

// purpose: removes a memory block from an arbitrary place in free_list and sets
//          is_free to false
// block: the block_header of the block to remove
static void __blkmngr_remove_from_free_list(block_header* block) {
    list_header* blk = (list_header*)block;
    if (is_end_of_list(blk)) {
        // ... -> [next] --> [next] -╮
        // ... -- [prev] <-- [prev] <╯
        //                     |
        //                     v
        // ... -> [next] -╮
        // ... -- [prev] <╯    
        //                ╭> [next] -╮
        //                ╰- [prev] <╯
        blk->prev->next = blk->prev;
    } else {
        // ... -> [next] --> [next] --> [next] -- ...
        // ... -- [prev] <-- [prev] <-- [prev] <- ...
        //                     |
        //                     v
        // ... -> [next] -------------> [next] -- ...
        // ... -- [prev] <------------- [prev] <- ...
        //                ╭> [next] -╮
        //                ╰- [prev] <╯
        blk->prev->next = blk->next;
        blk->next->prev = blk->prev;
    }
    blk->prev = blk;
    blk->next = blk;
    block->is_free = 0;
}

// purpose: finds the smalled free memory block that will acommodate request
// request_scale: request size as scale
// returns: best fitting block
static block_header* __blkmngr_find_fit(uint8_t request_scale) {
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
        __blkmngr_remove_from_free_list(curr);
        curr->is_free = 0;
    }

    return best_fit;
}

// purpose: splits an oversized block in half and adds one half to free list
//          as a scale-1 block
// block: a pointer to the block to be split
// returns: the other half as a scale-1 block
static void __blkmngr_split_block(block_header* block) {
    // reject small and NULL blocks
    if (!block || block->scale <= MIN_BLOCK_SCALE) return;

    // place new block_header 2^scale bytes away 
    block->scale--;
    block_header* new_block = (block_header*)((uint32_t)block + (1<<block->scale));

    // populate new_block header
    new_block->scale = block->scale;
    __blkmngr_add_to_free_list(new_block);
}

// purpose: merges a given block with it's buddy block (if buddy is free)
//          into a new scale+1 block. this process repeats until the entire
//          heap is merged or a buddy is full.
// block: a pointer to the block to potentiall merge
// returns: a pointer to the header of the resulting block (or original block
//            if the merge was unsuccessful)
static block_header* __blkmngr_coalesce_block(block_header* block) {
	// find buddy using block address & scale.
    block_header* curr_block = block;
    int32_t block_offset = (uint32_t)curr_block - HEAP_LOWER_BOUND;
    int32_t buddy_offset = (block_offset%(1<<(curr_block->scale+1))) ? \
                            -(1<<curr_block->scale) : (1<<curr_block->scale);
    block_header* buddy = (block_header*)((uint32_t)curr_block + buddy_offset);

    // repeatedly attempt a merge
    while(buddy->is_free && buddy->scale == curr_block->scale \
            && curr_block->scale < MAX_BLOCK_SCALE){
        
        // remove buddy from free list and update curr_block to point at 
        // whichever block header is earlier in memory.
        __blkmngr_remove_from_free_list(buddy);
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

// pupose: constructs the initial state of the heap: a single, contiguous block
// heap_addr: a pointer the LSB of the heap to initialize. 
void init_heap(void* heap_addr) {
    current_brk = heap_addr;

    // populate free_list such that each entry points to itself.
    for (uint8_t i = MIN_BLOCK_SCALE; i <= MAX_BLOCK_SCALE; i++) {
        free_list[i].prev = &free_list[i];
        free_list[i].next = &free_list[i];
    }

    // by default, the heap will be a single, max scale block.
    sbrk(1);
}

// purpose: allocates a section of memory dynamically
// request_size: the amount of memory (in bytes) needed
// returns: a pointer to the first free byte
void* allocate(size_t request_size) {
    // reject invalid reqests
    if (request_size > (1<<MAX_BLOCK_SCALE)-sizeof(block_header) \
       || !request_size) return NULL;

    uint32_t request_scale = max(size_to_scale(request_size), MIN_BLOCK_SCALE);
    block_header* block = __blkmngr_find_fit(request_scale);

    if (block) {
        // split block if too large
        while (block->scale > request_scale) {
            __blkmngr_split_block(block);
        }
    } else {
        // if block cannot be allocated, increase heap size and retry
        int8_t ret = sbrk(1);
        if (ret == -1) return NULL;
        block = __blkmngr_find_fit(request_scale);
    }

    // return the first free byte after the block_header. 
    return block+1;
}

// purpose: returns a previously allocated block of memory back into the
//          available pool.
// data: a pointer to the first free byte after a block_header. (this should be
//       the pointer returned by allocate().)
// returns: 0 on success, 1 on failure
uint8_t free(void* data) {
	// reject empty blocks
    if (!data) return 1;
    block_header* block = (block_header*)data-1;
	if ((block_header*)block->list.next != block || 
        (block_header*)block->list.prev != block) {
        
        char* str = "\nBLOCK DATA CORRUPTED. failed to free";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        return 1;
    }

    // merge and free block
	block = __blkmngr_coalesce_block(block);
    __blkmngr_add_to_free_list(block);

    // if freed block is max scale and the last block on the heap, call sbrk
    // to shrink heap.
    void* block_end = (void*)block+(1<<MAX_BLOCK_SCALE);
    if (block->scale == MAX_BLOCK_SCALE && current_brk == block_end) sbrk(-1);
    return 0;
}

// purpose: moves the current_brk according to the provided address. 
//          if addr > current_brk, (a request to grow the heap) then new max
//          scale blocks will be created until the address is within the heap.
//          if addr < current_brk, (a request to shrink the heap) then blocks
//          will be destroyed until the address is outside the heap.
// addr: the provided address to move the brk.
// returns: 0 on success, -1 on failure
int8_t brk(void* addr) {
    if (addr > current_brk){
        // repeatedly grow until requested addr is reached
        while (addr > current_brk) {
            // TODO: verify the space can be allocated
            
            // create new, max sized block
            block_header* big_block = (block_header*)current_brk;
            big_block->scale = MAX_BLOCK_SCALE;
            __blkmngr_add_to_free_list(big_block);

            // adjust current_break to first free byte afterward
            current_brk += (1<<MAX_BLOCK_SCALE);
        }
        return 0;
    } else {
        // repeatedly shrink heap if last block is max scale and free until
        // requested addr is reached
        while (addr < current_brk) {
            block_header* block = current_brk - (1<<MAX_BLOCK_SCALE);

            // exit with failure if an allocation is encountered
            if (block->scale != MAX_BLOCK_SCALE || !block->is_free) return -1;
            
            // remove empty block and move brk
            __blkmngr_remove_from_free_list(block);
            current_brk -= (1<<MAX_BLOCK_SCALE);
        }
        return 0;
    }
}

// purpose: adjusts current_brk by a relative amount of max scale blocks up or down.
// inc: amount to increment up or down
// returns: 0 on success, -1 on failure
int8_t sbrk(int32_t inc) {
    if (!inc) return -1;
    else return brk(current_brk + (inc<<MAX_BLOCK_SCALE));
}

// ----- FOR DEBUGGING ------
// void print_free_counts(){
//     terminal_writestring("free_block counts:\n");
//     for (uint8_t i = MIN_BLOCK_SCALE; i <= MAX_BLOCK_SCALE; i++) {
//         uint32_t count = 0;
//         list_header* curr = &free_list[i];
//         while (curr->next != curr)
//         {
//             count++;
//             curr = curr->next;
//         }
//         char c[2] = {(char)count+48, '\0'};
//         char c2[2] = {i>=10 ? (char)i+'A'-10 : (char)i+'0', '\0'};
//         terminal_writestring(c);
//         terminal_writestring(" free blocks of scale ");
//         terminal_writestring(c2);
//         terminal_writestring("\n");  
//     } 
//     char buf[18];
//     addr_to_string(buf, (uintptr_t)current_brk);
//     terminal_writestring("brk at ");
//     terminal_writestring(buf);
//     terminal_writestring("\n");  
// }


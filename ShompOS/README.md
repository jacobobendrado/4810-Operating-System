# Module 3: Memory Management

## Overview
This module extends ShompOS with a basic binary buddy allocator and deallocator. Regions of memory, known as blocks, will start large and be split as needed to handle smaller requests. Each block includes a block_header struct in the least significant bytes which gives information about the following block such as scale and whether or not is it currently allocated. 

## Key Topics
- The Heap
- Dynamic Memory Management

## Prerequisites
- Completed Kernel & IO Modules
- i686-elf cross-compiler toolchain
- QEMU for system emulation
- Make build system
- Basic understanding of structs and pointers 

## Quick Start
```bash
# Clone the repository (if you haven't already)
git clone https://github.com/jacobobendrado/4810-Operating-System.git
# Switch to the IO module branch
git switch memory_module
# Navigate to the project directory
cd ShompOS
# Run the OS in QEMU
make run
```
If everything worked, the behavior of the system will be initially identical to io_module. To try out the allocator, press a number key between `4` and `9`. To see current free blocks, press `p`.

## Key Files
- `src/memory/heap.c` - Handles allocation and free requests
- `inc/memory/heap.h` - Defines block_header struct
- `inc/fake_libc/fake_libc.h` - Defines list_header struct

## Key Terms
- `Block` - A region of memory for allocation. Will always be a power of 2 bytes and start with a 9 byte block_header struct.
- `Buddy` - Each block will have exactly one buddy. That is the "other half" that makes up a larger block and is created when said larger block is split. 
- `Size` - The size of an allocation request is the requested number of bytes.
- `Scale` - The scale of a blocks is its size as 2^scale bytes. Requests will be rounded up to the nearest scale.

## Implementation Details
- In short, when a request for memory comes in, the allocator will round the request up to the nearest scale and return a pointer to the first non-header byte of an appropriately sized block. 
- If no blocks of the requested scale are free, a larger block will be continually split in half until the appropriate scale is achieved. When a block is split in half, the two resulting blocks are called "buddies". One will be allocated and the other added to the free_list.
- When a block is eventually freed, it will check the status of it's buddy. If free, the two will be coalesced into a larger block which will then check it's buddy. This process will repeat until either the entire heap is returned to the inital state (one massive block) or some block's buddy is still allocated. 


## Helpful Resources and Further Reading
[Intel 64 and IA-32 Architectures Software Developer’s Manual](https://cdrdv2-public.intel.com/825743/325462-sdm-vol-1-2abcd-3abcd-4.pdf) Vol. 3A, Chapter 3 and 4 in particular
[Programming from the Ground Up - Johnathon Barlett](http://nongnu.askapache.com/pgubook/ProgrammingGroundUp-1-0-booksize.pdf#page163)
[Physical Page Allocation - Kernel.org](https://www.kernel.org/doc/gorman/html/understand/understand009.html)
[Memory Management Unit - OSDev](https://wiki.osdev.org/Memory_Management_Unit)
[Paging - OSDev](https://wiki.osdev.org/Paging) 
- [1.1] [Page Tables - OSDev](https://wiki.osdev.org/Page_Tables)
- [1.2] [Page Frame Allocation - OSDev](https://wiki.osdev.org/Page_Frame_Allocation)


## Future Improvements
- Memory leak prevention
- Use after free protection
- Multiple small allocations from the same process housed in one larger block
- Paging
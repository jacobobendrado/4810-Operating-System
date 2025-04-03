// elf.c
// ELF implementation
// Cedarville University 2024-25 OSDev Team

#include <elf.h>
#include <ramfs.h>
#include <heap.h>
#include <string.h>

#define STACK_SIZE_DEFAULT 1000

processID init_elf(ramfs_file_t* f) {
    int rc = is_readable(f);
    if (rc) {
        return ELF_ERROR;
    }


    Elf32_Ehdr *elfHeader = (Elf32_Ehdr*)f->data;


    Elf32_Phdr *pHeaders = (Elf32_Phdr*)(f->data + elfHeader->e_phoff);
    void *textSpace;
    uint32_t size = 0;
    Elf32_Addr min_vaddr = -1;

    Elf32_Phdr *header = &pHeaders[0];

    // Read every program header to determine what the lowest virtual address
    // is and how much space is needed to be allocated
    for (int i = 0; i < elfHeader->e_phnum; i++) {

        header = &pHeaders[i];

        // If LOAD,
        if (header->p_type != PROGRAM_TYPE_LOAD && header->p_memsz > 0) {
            continue;
        }

        if (header->p_vaddr < min_vaddr) {
            if (size > 0) {
                size = size + (min_vaddr - header->p_vaddr);
            }
            else {
                size = header->p_memsz;
            }
            min_vaddr = header->p_vaddr;
        }

        if (header->p_vaddr + header->p_memsz > min_vaddr + size) {
            size = (header->p_vaddr + header->p_memsz) - min_vaddr;
        }
    }

    textSpace = allocate(size);

    if (!textSpace) {
        return ELF_ERROR;
    }

    for (int i = 0; i < elfHeader->e_phnum; i++) {

        header = &pHeaders[i];

        // If LOAD,
        if (header->p_type != PROGRAM_TYPE_LOAD && header->p_memsz > 0) {
            continue;
        }

        // Copy segment data from file
        memcpy(textSpace, f->data + header->p_offset, header->p_filesz);
    
        // Zero out rest of segment
        memset(textSpace + header->p_filesz, '\0', header->p_memsz - header->p_filesz);

    }

    // Create stack
    void *stackSpace = allocate(STACK_SIZE_DEFAULT);
    if (!stackSpace) {
        free(textSpace);
        return ELF_ERROR;
    }

    return init_process(textSpace + elfHeader->e_entry - min_vaddr, stackSpace);
}


int is_readable(ramfs_file_t* f) {
    Elf32_Ehdr *elfHeader = (Elf32_Ehdr*)f->data;
    if (elfHeader->e_ident[0] != '\x7F' ||
           elfHeader->e_ident[1] != 'E' ||
           elfHeader->e_ident[2] != 'L' ||
           elfHeader->e_ident[3] != 'F' ) {
        return NOT_ELF_FILE;
    }

    if  (elfHeader->e_ident[4] != ELFCLASS32) {
        return ELF_UNREADABLE;
    }

    // Check for endianness; not certain if this is needed
    if  (elfHeader->e_ident[5] != E_LITTLE_ENDIAN) {
        return ELF_UNREADABLE;
    }

    if (elfHeader->e_type != E_TYPE_EXEC) {
        return ELF_UNREADABLE;
    }

    if (elfHeader->e_machine != E_MACHINE_386 && elfHeader->e_machine != E_MACHINE_NONE) {
        return ELF_UNREADABLE;
    }

    return 0;

}
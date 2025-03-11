// elf.c
// ELF implementation
// Cedarville University 2024-25 OSDev Team

#include <elf.h>
#include <heap.h>
#include <string.h>

#define STACK_SIZE_DEFAULT 1000

int init_elf(ramfs_file_t* f) {
    int rc = is_readable(f);
    if (rc) {
        return ELF_ERROR;
    }

    Elf32_Ehdr *elfHeader = f->data;

    Elf32_Phdr *pHeaders = f->data + elfHeader->e_phoff;
    void *textSpace; //TODO: figure out doing it with multiple parts

    // Read every program header
    for (int i = 0; i < elfHeader->e_phnum; i++) {

        Elf32_Phdr *header = &pHeaders[i];

        // If LOAD,
        if (header->p_type != PROGRAM_TYPE_LOAD) {
            continue;
        }
    
        // Allocate virtual memory for each segment (if gt 0)
        textSpace = allocate(header->p_memsz);

        if (!textSpace) {
            return ELF_ERROR;
        }
    
        // Copy segment data from file
        memcpy(textSpace, f->data + header->p_offset, header->p_filesz);
    
        // Zero out rest of segment
        memset(textSpace + header->p_filesz, '\0', header->p_memsz - header->p_filesz);

    }

    // Create stack?
    void *stackSpace = allocate(STACK_SIZE_DEFAULT);
    if (!stackSpace) {
        free(&textSpace);
        return ELF_ERROR;
    }

    //TODO: Integrate process
    // return init_process(textSpace + elfHeader->e_entry - header->p_vaddr, stackSpace + STACK_SIZE_DEFAULT)

    return 1;
}


int is_readable(ramfs_file_t* f) {
    Elf32_Ehdr *elfHeader = f->data;
    if (elfHeader->e_ident[0] != '\x7F' ||
           elfHeader->e_ident[1] != 'E' ||
           elfHeader->e_ident[2] != 'L' ||
           elfHeader->e_ident[3] != 'F' ) {
        return NOT_ELF_FILE;
    }

    if  (elfHeader->e_ident[4] != ELFCLASS32) {
        return ELF_UNREADABLE;
    }

    //TODO: Endianness?

    if (elfHeader->e_type != E_TYPE_EXEC) {
        return ELF_UNREADABLE;
    }

    if (elfHeader->e_machine != E_MACHINE_386 && elfHeader->e_machine != E_MACHINE_NONE) {
        return ELF_UNREADABLE;
    }

    return 0;

}
#pragma once

#include <stddef.h>
#include <stdint.h>

enum process_status {WAITING, ACTIVE, STOPPED}


typedef struct _process_struct {
    uint8_t PID;
    process_status status
    context_struct context;
    process_struct* parent_process;
    

    // uint8_t max_fd;
    // file_descriptor* fd_list;

} process_struct;


typedef struct _context_struct {
    uint32_t EIP;

    uint32_t ESP;
    uint32_t EBP;

    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    
    uint32_t ESI;
    uint32_t EDI;

    uint32_t EFLAGS;
} context_struct;


uint8_t get_next_PID();
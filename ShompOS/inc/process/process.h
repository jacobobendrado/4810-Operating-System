#pragma once

#include <stddef.h>
#include <stdint.h>
#include <process/context_switch.h>

// Limit of how many processes can run at once
#define MAX_PROCESS 0x8

typedef enum {
    ACTIVE, // running currently
    WAITING, 
    STOPPED,
    SPAWNED
} process_status;

typedef struct _process_struct {
    context_struct context;
    uint8_t PID;

    process_status status;
    // struct _process_struct* parent_process;
    // uint8_t max_fd;
    // file_descriptor* fd_list;
} process_struct;


uint8_t get_next_PID();
uint8_t init_process(void* entry_point, void* stack);
void switch_process(uint8_t PID);


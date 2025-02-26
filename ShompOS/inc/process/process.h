#pragma once

#include <stddef.h>
#include <stdint.h>
#include <process/context_switch.h>

typedef uint32_t processID;

// Limit of how many processes can run at once
#define MAX_PROCESS 0x8
// chosen arbitrarily, i like the word BLOB.
#define MAX_PID 0xB10B 

typedef enum {
    STOPPED, // dead, will not run again
    ACTIVE,  // running currently
    WAITING, // waiting for its turn on the CPU 
    SPAWNED  // initialized but not yet scheduled
} process_status;

typedef struct _process_struct {
    context_struct context;
    processID PID;
    process_status status;
    void* enty_point;
    struct _process_struct* parent_process;
    uint32_t wait_time;
    // uint8_t max_fd;
    // file_descriptor* fd_list;
} process_struct;


processID init_process(void* entry_point, void* stack, processID parent_PID);
void switch_process(processID PID);
void switch_process_from_queue();
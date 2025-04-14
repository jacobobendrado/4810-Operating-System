// process.h
// Process management
// Cedarville University 2024-25 OSDev Team

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
    uint32_t wait_time;
    
    // for fork()
    // void* entry_point;
    
    // for per-process FD
    // uint8_t max_fd;
    // file_descriptor* fd_list;
} process_struct;

extern processID active_pid;

processID init_process(void* entry_point, void* stack);
void kill_process(processID PID);
void switch_process(processID PID);
void switch_process_from_queue();
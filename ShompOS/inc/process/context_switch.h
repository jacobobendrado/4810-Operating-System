#pragma once

// used by context_switch() to store and restore a process' state while 
// multi-processing. 
// 12 bytes
typedef struct _context_struct {
    void* ESP;
    void* stack_top;
    void* stack_bottom;
} context_struct;

extern void context_switch(context_struct* current, context_struct* next);
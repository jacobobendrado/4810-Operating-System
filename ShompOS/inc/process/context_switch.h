#pragma once

typedef struct _context_struct {
    void* ESP;
    void* stack_top;
} context_struct;

extern void context_switch(context_struct* current, context_struct* next);
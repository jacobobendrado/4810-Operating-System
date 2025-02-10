#include <process/process.h>
#include <process/context_switch.h>
#include <kernel/kernel.h>

process_struct proc_table[MAX_PROCESS];
uint8_t active_pid = 0;

uint8_t next_pid = 0x00;
uint8_t get_next_PID(){
    next_pid++;
    return next_pid;
};

uint8_t init_process(void* entry_point, void* stack_top){
    uint8_t PID = get_next_PID();
    process_struct* proc = &proc_table[PID];
    proc->PID = PID;
    // proc->parent_process = proc;
    proc->status = SPAWNED;
    
    *((uint32_t*)stack_top-0x0) = (uint32_t)entry_point;
    *((uint32_t*)stack_top-0x1) = (uint32_t)stack_top;
    *((uint32_t*)stack_top-0x2) = 0xAAAA; // EAX
    *((uint32_t*)stack_top-0x3) = 0xCCCC; // ECX
    *((uint32_t*)stack_top-0x4) = 0xDDDD; // EDX
    *((uint32_t*)stack_top-0x5) = 0xBBBB; // EBX
    *((uint32_t*)stack_top-0x6) = (uint32_t)stack_top-0x4; // ESP
    *((uint32_t*)stack_top-0x7) = (uint32_t)stack_top-0x4; // EBP
    *((uint32_t*)stack_top-0x8) = 0x5151; // ESI
    *((uint32_t*)stack_top-0x9) = 0xD1D1; // EDI
    *((uint32_t*)stack_top-0xA) = 0x0202; // EFLAGS      

    context_struct* cntx = &proc->context;
    cntx->ESP = (uint32_t*)stack_top-0xA;
    cntx->stack_top = stack_top;

    return PID;
    // context_switch(&proc_table[0].context, cntx);
};


void switch_process(uint8_t PID){
    // TODO: sane-atize inputs

    process_struct* old_proc = &proc_table[active_pid];
    process_struct* new_proc = &proc_table[PID];
    
    old_proc->status = WAITING;
    new_proc->status = ACTIVE;
    active_pid = PID;

    context_switch(&old_proc->context, &new_proc->context);
}

// need to define stack regigons
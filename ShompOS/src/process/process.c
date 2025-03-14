#include <process/process.h>
#include <process/context_switch.h>
#include <kernel/kernel.h>

process_struct proc_table[MAX_PROCESS];
processID active_pid = 0;
processID next_pid = 0;

// purpose: gets the next valid PID. CURRENTLY DOES NOT CHECK IF PID IS ALREADY
//          IN USE
// returns: the next valid PID 
processID get_next_PID() {
    if (next_pid++ >= MAX_PID) next_pid = 1;
    return next_pid;
}

// purpose: finds the next open spot in the proc_table
// returns: a pointer to the open slot, if all slots are full, returns NULL
process_struct* reserve_proc_table_slot() {
    process_struct* proc = NULL;

    for (uint8_t i = 0; i < MAX_PROCESS; i++){
        if (proc_table[i].status == STOPPED){
            proc = &proc_table[i];
            break;
        }
    }

    return proc;
}

// purpose: finds proc_table entry associated with a PID
// PID: the process PID to find
// returns: a pointer to the process_struct of requested process
process_struct* get_process(processID PID) {
    process_struct* proc = NULL;

    for (uint8_t i = 0; i < MAX_PROCESS; i++){
        if (proc_table[i].PID == PID && proc_table[i].status != STOPPED){
            proc = &proc_table[i];
            break;
        }
    }

    return proc;
}

// purpose: sets up inital stack state for a new process. the new process is
//          left in the SPAWNED status and will not be scheduled until 
//          explicitly asked.  
// entry_point: a pointer to the instruction that begins execution
// stack_top: a pointer to the highest valid address to use in new process' 
//            stack
// parent_PID: PID of the parent process
// returns: the PID of the newly created process
processID init_process(void* entry_point, void* stack_top, processID parent_PID) {
    processID PID = get_next_PID();
    process_struct* proc = reserve_proc_table_slot();

    if (proc == NULL) {
        // TODO: actually handle the error
        terminal_writestring("CANNOT RESERVE PROCESS");
    }
    
    // setup initial stack conditions. these values will be popped into the
    // general purpose registers on process startup. the only important values
    // are the stack pointers and EFLAGS. everything else is placeholder.
    *((uint32_t*)stack_top-0x0) = (uint32_t)entry_point;   // after POPAL, the CPU will RET to this addr.
    *((uint32_t*)stack_top-0x1) = (uint32_t)stack_top;     // real start of the stack
    *((uint32_t*)stack_top-0x2) = 0xAAAA;                  // EAX
    *((uint32_t*)stack_top-0x3) = 0xCCCC;                  // ECX
    *((uint32_t*)stack_top-0x4) = 0xDDDD;                  // EDX
    *((uint32_t*)stack_top-0x5) = 0xBBBB;                  // EBX
    *((uint32_t*)stack_top-0x6) = (uint32_t)stack_top-0x4; // ESP
    *((uint32_t*)stack_top-0x7) = (uint32_t)stack_top-0x4; // EBP
    *((uint32_t*)stack_top-0x8) = 0x0E51;                  // ESI
    *((uint32_t*)stack_top-0x9) = 0x0ED1;                  // EDI
    *((uint32_t*)stack_top-0xA) = 0x0202;                  // EFLAGS (DO NOT CHANGE. other values will crash lol)

    // set up the context_struct
    context_struct* cntx = &proc->context;
    cntx->ESP = (uint32_t*)stack_top-0xA;
    cntx->stack_top = stack_top;

    // set up the process_struct
    proc->PID = PID;
    proc->status = SPAWNED;
    proc->enty_point = entry_point;
    proc->parent_process = get_process(parent_PID);
    proc->wait_time = 0;

    return PID;
};

// purpose: duplicates the process associated with provided PID
// PID: the PID of the process to duplicate 
// returns: the PID of the newly spawned process
processID fork(processID PID) {

    return 0xFFFF;
}

// purpose: performs a context switch to the process associated with a PID
// PID: the PID of the process to switch to
void switch_process(processID PID) {
    // TODO: sane-atize inputs

    process_struct* old_proc = get_process(active_pid);
    process_struct* new_proc = get_process(PID);
    
    // if (new_proc == NULL || old_proc == NULL) {
    //     terminal_writestring("UH OH");
    //     return;
    // }
    // if (new_proc->status == STOPPED) terminal_writestring("PROCESS NOT LIVE");

    old_proc->status = WAITING;
    new_proc->status = ACTIVE;
    active_pid = PID;

    context_switch(&old_proc->context, &new_proc->context);
}


// purpose: performs a context switch according to active scheduling algorithm. 
void switch_process_from_queue() {
    process_struct* proc = &proc_table[0];

    for (uint8_t i = 0; i < MAX_PROCESS; i++){
        if (proc_table[i].status == WAITING || proc_table[i].status == SPAWNED){
            if (proc_table[i].wait_time++ > proc->wait_time){
                proc = &proc_table[i];
            }
        }
    }

    proc->wait_time = 0;
    switch_process(proc->PID);
}
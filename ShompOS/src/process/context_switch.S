.global context_switch
context_switch:
    # save old base pointer and set up new base pointer
    pushl %ebp     
    movl %esp, %ebp

    # save context to current stack
    pushal
    pushfl

    # get current process_struct pointer and save current ESP to it
    movl 8(%ebp), %ebx
    movl %esp, (%ebx)          

    # get new process_struct pointer and load it's ESP
    movl 12(%ebp), %ebx
    movl (%ebx), %esp
    
    # restore context from new stack
    popfl
    popal
    
    # pop the saved EIP and continue execution
    leave
    ret
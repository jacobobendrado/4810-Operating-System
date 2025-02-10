.global context_switch
context_switch:
    pushl %ebp              # Save old base pointer
    movl %esp, %ebp        # Set up new base pointer

    # Save current context
    pushal                      # Push all general purpose registers
    pushfl                      # Push FLAGS register

    
    # Get current process struct pointer (first argument)
    movl 8(%ebp), %ebx
    movl %esp, (%ebx)          # Save current ESP to process struct
    
    # Get new process struct pointer (second argument)
    movl 12(%ebp), %ebx
    movl (%ebx), %esp          # Load new ESP
    
    # Restore new context
    popfl                       # Restore FLAGS
    popal                       # Restore all general purpose registers
    
    leave
    ret                        # This will pop the saved EIP and continue execution

.global init_process_stack
init_process_stack:
    
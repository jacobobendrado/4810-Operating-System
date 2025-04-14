# mnt/inc/syscalls.S
# Library to wrap syscalls
# Cedarville University 2024-25 OSDev Team

.global do_syscall

# TODO: Surely there's a better way to do this...
do_syscall:
    # Save necessary registers
    push %ebp
    push %edi
    push %esi
    push %ebx

    # Move from stack to registers
    movl 20(%esp), %eax
    movl 24(%esp), %ebx
    movl 28(%esp), %ecx
    movl 32(%esp), %edx
    movl 36(%esp), %esi
    movl 40(%esp), %edi
    movl 44(%esp), %ebp

    int $0x80

    pop %ebx
    pop %esi
    pop %edi
    pop %ebp
    ret
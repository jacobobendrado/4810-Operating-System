
.global do_syscall


do_syscall:
    push %ebp
    push %edi
    push %esi
    push %ebx
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

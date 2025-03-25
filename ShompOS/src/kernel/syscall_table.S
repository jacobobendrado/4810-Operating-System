
.extern syscall_exit

sys_table:
    .long 0
    .long syscall_exit
    .space 1528

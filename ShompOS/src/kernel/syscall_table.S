# syscall_table.S
# Table of syscall function pointers
# Cedarville University 2024-25 OSDev Team

.extern syscall_exit

sys_table:
    .long 0
    .long syscall_exit
    .space 1528

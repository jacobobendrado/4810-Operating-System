# syscall_table.S
# Table of syscall function pointers
# Cedarville University 2024-25 OSDev Team

.extern syscall_exit
.extern ramfs_read
.extern ramfs_write

.extern terminal_putentryat

sys_table:
    .long 0 # intenionally blank
    .long syscall_exit
    .long 0 # fork()
    .long ramfs_read
    .long ramfs_write
    .long 0 # open()
    .long 0 # close()
    .long terminal_putentryat
    .space 1516

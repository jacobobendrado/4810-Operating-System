# debug.gdb
# File for GDB to connect to QEMU
# Cedarville University 2024-25 OSDev Team

# Necessary for integration with QEMU
target remote localhost:1234

# Add other commands to run when initializing.
# Ex. layout src
#     break kernel_main
#     continue
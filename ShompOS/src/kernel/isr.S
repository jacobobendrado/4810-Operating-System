# isr.S
# Interrupt Service Routine implementations
# Cedarville University 2024-25 OSDev Team

# Tutorial built from https://wiki.osdev.org/Interrupts_Tutorial

.macro isr_err_stub num
isr_stub_\num:
    call exception_handler
    iret
.endm

.macro isr_no_err_stub num
isr_stub_\num:
    call exception_handler
    iret
.endm

.macro unrecoverable_error
    call kill_process
    iret
.endm

.extern exception_handler
.extern kill_process
isr_no_err_stub 1  # This could be useful (debug)
isr_no_err_stub 3  # This could be useful (breakpoint)
isr_no_err_stub 6  # Probably not
isr_no_err_stub 7  # Probably not
isr_no_err_stub 9  # MIA
isr_err_stub    13 # Probably not
isr_err_stub    14 # Yes after memory is implemented
isr_no_err_stub 15 # Reserved
isr_err_stub    17 # Probably not
isr_no_err_stub 18 # Probably not
isr_no_err_stub 20 # Probably not
isr_no_err_stub 21 # Probably not
isr_no_err_stub 22 # Reserved
isr_no_err_stub 23 # Reserved
isr_no_err_stub 24 # Reserved
isr_no_err_stub 25 # Reserved
isr_no_err_stub 26 # Reserved
isr_no_err_stub 27 # Reserved
isr_no_err_stub 28 # Probably not
isr_no_err_stub 29 # Probably not
isr_err_stub    30 # Probably not
isr_no_err_stub 31 # Reserved

.extern handle_div_by_zero
isr_0:
    pushal
    cld
    call handle_div_by_zero

    # Increment the instruction pointer for returning
    # to skip the div instruction.
    # Should be changed later to kill the process
    movl 0x20(%esp), %edx
    addl $2, %edx
    movl %edx, 0x20(%esp)
    popal
    iret

isr_2:
    unrecoverable_error

isr_4:
    unrecoverable_error

isr_5:
    unrecoverable_error

isr_8:
    unrecoverable_error

isr_10:
    unrecoverable_error

isr_11:
    unrecoverable_error

isr_12:
    unrecoverable_error

isr_16:
    unrecoverable_error

isr_19:
    unrecoverable_error

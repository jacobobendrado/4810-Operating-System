# boot.S
# Initial functions to boot and link to C code
# Cedarville University 2024-25 OSDev Team

# multiboot magic. taken from OSDev.wiki
.set ALIGN,    1<<0           
.set MEMINFO,  1<<1            
.set FLAGS,    ALIGN | MEMINFO  
.set MAGIC,    0x1BADB002      
.set CHECKSUM, -(MAGIC + FLAGS) 


.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .text

.include "src/kernel/gdt.S"
.include "src/kernel/ist.S"
.include "src/kernel/syscall_table.S"

# these functions will be called from kernel.c
.global start
.global load_gdt
.global load_idt
.global syscall_handler
.global keyboard_handler
.global clock_handler
.global ioport_in
.global ioport_out
.global enable_interrupts

# these functions are in kernel.c and will 
# be called in assembly
.extern kernel_main            
.extern handle_keyboard_interrupt
.extern handle_clock_interrupt

# loads the gdt_descriptor found in gdt.s
load_gdt:
    lgdt gdt_descriptor
    ret

# when a C function passes data, those 
# variables will be stored on the stack.
# the last entry will be the 32 bit return
# address. so we count 4 bytes to get the
# first parameter: the IDT address
load_idt:
    movl 4(%esp), %edx
    lidt (%edx)
    ret

# guess.
enable_interrupts:
    sti
    ret

# handle syscalls
syscall_handler:
    pushal
    sall $2, %eax
    addl sys_table, %eax
    call *%eax

# pushes all registers before calling C function. this is to prevent us from
# losing our context when we return from the interrupt handler
keyboard_handler:
    pushal
    cld
    call handle_keyboard_interrupt
    popal
    iret


clock_handler:
    pushal
    cld
    call handle_clock_interrupt
    popal
    iret




# reads a byte from an IO port (address
# stored in dx) into al 
ioport_in:
    movl 4(%esp), %edx  
    inb %dx, %al        
    ret

# writes a byte to an IO port (address
# stored in dx) from al 
ioport_out:
    movl 4(%esp), %edx  
    movl 8(%esp), %eax  
    outb %al, %dx
    ret


start:
    lgdt gdt_descriptor
    ljmp $CODE_SEG, $.setcs  
.setcs:
    movw $DATA_SEG, %ax      
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movl $stack_top, %esp  
    cli                      

    #movl $stack_top, %esp
    call kernel_main
    hlt

.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB

stack_top:
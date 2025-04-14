// boot.h
// Header file for assembly files run at startup
// Cedarville University 2024-25 OSDev Team

#pragma once 

#include <stdint.h>

// ----- Assembly functions -----
extern void load_gdt();
extern void syscall_handler();
extern void keyboard_handler();
extern void clock_handler();
extern char ioport_in(uint16_t port);
extern void ioport_out(uint16_t port, uint8_t data);
extern void load_idt(uint32_t* idt_address);
extern void enable_interrupts();
extern void* isr_stub_table[];
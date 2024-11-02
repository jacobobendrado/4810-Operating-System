# Module 2: Keyboard & Interrupt Module

## Overview
This module extends ShompOS with keyboard input and interrupt handling capabilities. It demonstrates setting up the Global Descriptor Table (GDT), handling keyboard interrupts, and managing basic hardware exceptions.

## Key Topics
- Interrupt Service Routines (ISR)
- Global Descriptor Table (GDT)
- Keyboard Interrupts
- Exception Handling

## Prerequisites
- Completed Kernel Module
- i686-elf cross-compiler toolchain
- QEMU for system emulation
- Make build system
- Basic understanding of x86 assembly

## Quick Start
```bash
# Clone the repository (if you haven't already)
git clone https://github.com/jacobobendrado/4810-Operating-System.git
# Switch to the IO module branch
git switch io_module
# Navigate to the project directory
cd ShompOS
# Run the OS in QEMU
make run
```
If everything worked, you should be able to see your keyboard inputs displayed in the screen. To try out the Divide by Zero Exception, press the `0` key.

## Key Files
- `kernel/isr.S` - Handles hardware interrupts and exceptions
- `kernel/keyboard_map.h` - Defines keyboard scan code mappings
- `kernel/kernel.c` - Where the handling and printing occurs
- `kernel/boot.S` - Where we get the data from the hardware

## Implementation Details
- Keyboard interrupt handling
- Basic scan code to ASCII conversion
- Example exception handlers for CPU faults

## Helpful Resources
- [PageKey - Basic Keyboard Driver](https://www.youtube.com/watch?v=YtnNX074jMU)
- [OSDev Wiki - GDT](https://wiki.osdev.org/GDT)
- [OSDev Wiki - ISRs](https://wiki.osdev.org/ISR)
- [OSDev Wiki - PS/2 Keyboard](https://wiki.osdev.org/PS2_Keyboard)
- [OSDev Wiki - Interrupts Tutorial](https://wiki.osdev.org/Interrupts_Tutorial)

## Future Improvements
- Shift key implementation
- Additional exception handlers
- Basic shell interface

# Module 1: Kernel Module

## Overview
This is a simple operating system kernel that prints "Hello, World!" to the screen. It's designed as a learning project to understand how operating systems work at the most basic level.

## Key Topics
- Bootloading with GRUB
- Virtual Machine testing
- Build system configuration

## Prerequisites
- Set up a [GCC Cross-Compiler](https://wiki.osdev.org/GCC_Cross-Compiler "GCC Cross-Compiler") for **i686-elf**
- QEMU for system emulation
- Make build system

Installation guides:
- [Cross-compiler setup guide](https://wiki.osdev.org/GCC_Cross-Compiler)
- [QEMU installation guide](https://www.qemu.org/download/#linux)
- Make usually comes with Linux/MacOS. On Windows, install via [WSL](https://docs.microsoft.com/en-us/windows/wsl/install)

## Quick Start
```bash
# Clone the repository (if you haven't already)
git clone https://github.com/jacobobendrado/4810-Operating-System.git
# Enter the repository directory
cd 4810-Operating-System
# Switch to the kernel module branch
git switch kernel_module
# Navigate to the project directory
cd ShompOS
# Run the OS in QEMU
make run
```
If everything worked, you should see "Hello, World!" in QEMU!

## Exiting QEMU
Sometimes, it's tricky to try to get out of QEMU when it's running. If the X in the top right corner isn't working, try Ctrl+Alt+G to take control, then click that again. Alternatively, you can send the interrput signal, Ctrl+C, in the terminal where you ran `make run`.

## Key Files
- `src/kernel/kernel.c` - Main kernel code
- `src/kernel/boot.h` - OS entrypoint and multiboot header
- `Makefile` - Build configuration

## Implementation Details
- Basic kernel initialization
- Screen output functionality
- GRUB bootloader integration

## Important Foundations
- [OSDev Wiki - Bare Bones Tutorial](https://wiki.osdev.org/Bare_Bones)
- Operating Systems basics
- x86 assembly basics
- How bootloaders work

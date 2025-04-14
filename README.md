# 4810-Operating-System - ShompOS

## Our Goal
- Learn as much as possible about operating systems, low-level programming, and computer hardware
- Build an operating system in C using the x86 architecture
- Structure the OS in independently runnable sections aimed as an educational resource

## Platform
- Language: C
- Architecture: x86 (32-bit)
- Assembly Syntax: AT&T (using GNU Assembler 'as')
- Emulator: QEMU
- Bootloader: GRUB
- Compiler: Custom GCC (i686-elf Cross Compiler)
- Output Format: ISO image

- Memory Model: "Binary Buddy"
- File System: In-memory File System

## Installation
- Set up a [GCC Cross-Compiler](https://wiki.osdev.org/GCC_Cross-Compiler "GCC Cross-Compiler") for **i686-elf**
- QEMU for system emulation
- Make build system

In the ShompOS/ directory, run `make run` to build the OS and run it.

Other helpful commands:
- `make clean`: Removes all build artifacts
- `make`: Builds the project
- `make run-iso`: Creates and runs iso

Under development during 2024-25 school year.

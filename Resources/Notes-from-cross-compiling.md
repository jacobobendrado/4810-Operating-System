# Cross-Compiling

### Defintitions
A cross-compiler is a compiler that runs on one platform but creates executables for a different platform.

Freestanding executing environments are independent of standard runtime and OS facitities.
In OS development, it's usually the kernel.

Hosted executing environments use a complete OS.
These are in user space.
When programming in C, the standard library already exists.

### Is it necessary?
It's only necessary if writing code on operating system that is not being developed, which we're doing.
It is possible to use the built-in compiler, but there's a lot that's assumed when using that, and it's best to simply make one for the target system.

### How complicated is it?
It's really not bad.
There's a lot of set up, especially if you're not developing on your own Linux machine, but after that, you simply build a subset of GCC's capabilities and have it target your OS.
Eventually, setting up an OS-specific toolchain might be required, but the basic tutorial should be sufficient for a while.

### What's a toolchain?
A toolchain is a set (chain) of tools that are used to create a programmed product (usually an executable). 
These are usually used in the world of cross-compiling.
A straightforward toolchain would be to call the compiler, then the assembler, and then the linker.

[Resource](https://stackoverflow.com/questions/43929201/what-is-a-compiler-toolchain)

Note: The command `gcc` is not just a compiler- it's a whole toolchain (I think)!
GCC stands for "GNU Compiler Collection."

### Making a toolchain
This is pretty low-level and not necessary right now.
There's a lot of things that you can do to change things, but we probably won't need to do it.

[Later Toolchain](https://wiki.osdev.org/OS_Specific_Toolchain)


### Random Notes
- If we make a C++ compiler, we would have to supply a C++ support library to the kernel to make the whole language work.
- We can define a preprocessor macro `__myos__` defined by the compiler.
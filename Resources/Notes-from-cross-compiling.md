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

### How complicated is it?
It's really not bad.
There's a lot of set up, especially if you're not developing on your own Linux machine, but after that, you simply build a subset of GCC's capabilities and have it target your OS.
Eventually, setting up an OS-specific toolchain might be required, but the basic tutorial should be sufficient for a while.
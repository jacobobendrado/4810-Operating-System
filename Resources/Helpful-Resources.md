# Helpful Resources

## Websites
- [OSDev](https://wiki.osdev.org/Expanded_Main_Page)

## Projects
- [MentOS](https://mentos-team.github.io/)
- [Making an OS x86 Youtube Series](https://www.youtube.com/watch?v=MwPjvJ9ulSc&list=PLm3B56ql_akNcvH8vvJRYOc7TbYhRs19M)
    - Notes are in Progress. See Weston
- [Build a 65c02-based computer from scratch YT series](https://youtu.be/LnzuMJLZRdU?si=9LvxP228M4_oDLGK)
    - Helpful resource for refreshing on computer hardware and assembly
    - Notes are in Progress. -MR
- [Write your own Operating System in 1 hour YT series](https://youtu.be/1rnA6wpF0o4?si=XApDFYEc0jnQWAvZ)
    - Written in C++, not C
- [BrokenThrorn Entertainment OS Development Series](http://www.brokenthorn.com/Resources/OSDevIndex.html)
    - If someone checks this out, please let us know
- [OS12: Basic Keyboard Driver (x86 Interrupts)](https://youtu.be/YtnNX074jMU?si=dnlJ8g2pHdyQxCb2)
- More projects can be found on the [OSDev Project page](https://wiki.osdev.org/Projects)


## Tools
#### QEMU
- A generic and open source machine emulator and virtualizer.
- To get a jumpstart on using QEMU, check out the tldr documentation [here](https://tldr.inbrowser.app/pages/common/qemu)
    - The official documentation can be found at [Qemu Documentation](https://www.qemu.org/docs/master/)
- Debug is easy since you can connect GDB to a virtual machine to debug code that runs on it, through QEMU's built-in GDB server.
    - See more about using GDB in QEMU [here](https://www.qemu.org/docs/master/system/gdb.html).
    - Also, check out Operating Systems: From 0 to 1. In the Chapter on bootloaders (page 197), it walks through and example.

#### as
- Portable GNU assembler. Primarily intended to assemble output from `gcc` to be used by `ld`.
- To get a jumpstart on using `as`, check out the tldr documentation [here](https://tldr.inbrowser.app/pages/linux/as)

#### make
- Task runner for targets described in Makefile. Mostly used to control the compilation of an executable from source code.
- To get a jumpstart on using `make`, check out the tldr documentation [here](https://tldr.inbrowser.app/pages/common/make)


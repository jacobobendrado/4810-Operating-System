
## From the Author
This book helps you gain the foundational knowledge required to write an operating system from scratch. Hence the title, 0 to 1.

After completing this book, at the very least you will learn:
- How to write an operating system from scratch by reading hardware datasheets. In the real world, it works like that. You won't be able to consult Google for a quick answer.
- A big picture of how each layer of a computer is related to the other, from hardware to software.
- Write code independently. It's pointless to copy and paste code. Real learning happens when you solve problems on your own. Some examples are given to kick start, but most problems are yours to conquer. However, the solutions are available online for you to examine after giving it a good try.
- Linux as a development environment and how to use common tools for low-level programming.
- x86 assembly in-depth.
- How a program is structured so that an operating system can run.
- How to debug a program running directly on hardware with gdb and QEMU.
- Linking and loading on bare metal x86_64, with pure C. No standard library. No runtime overhead.

## Notes and Quotes
### Preface
- This book was made to be as self-contained as possible so that you can spend more time learning and less time guessing or seeking out information on the Internet (pg. ii-iii)
- Write code independently. It is pointless to copy and paste code. Real learning happens when you solve problems on your own (pg. iv)
### Chapter 1: Domain documents
- The bulk of software design and implementation depends upon the knowledge of the problem domain. The better understood the domain, the higher quality the software can be (pg. 7). Plan ahead and know the ins and outs of the domain your are implementing
### Chapter 2: From hardware to software: Layers of abstraction
- Understand assembly language is crucial for low-level programming domains, even to this day (pg. 21)
- A computer essentially implements this process:
	- *Fetch* an instruction from a storage device 
	- *Decode* the instruction
	- *Execute* the instruction (pg. 23)
- The higher level a programming language is, the more distant it comes from the hardware (pg. 25)
- This is the reason why C is usually a language of choice for writing an operating system, since C is just a thin wrapper of the underlying hardware, making it easy to understand how exactly a hardware device runs when executing a certain piece of C code (pg. 25-26)
- Recurring patterns are the key to abstraction. Recurring patterns are why abstraction works (pg. 27)
- If the domain knowledge cannot be extracted, then the software cannot be further developed (pg. 30). This is key for us
- If a programmer knows absolutely nothing about hardware, then it is impossible to read and write operating system code in C, even if he could have 20 years of writing application C code (pg. 30). Understanding the domain is the first priority!
### Chapter 3: Computer Architecture
- In a CPU, many OS concepts are already implemented directly in hardware, e.g. task switching, paging. A kernel programmer needs to know how to use the hardware features, to avoid duplicating such concept in software, thus wasting computer resources (pg. 41)
- Learning to program the CPU is the most important, as it is the component present in any computer (pg. 45)
### Chapter 4: x86 Assembly and C
- Knowing the difference between 32-bit and 64-bit is crucial for writing kernel code (pg. 51)

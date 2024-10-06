# Global Descriptor Table
The Global Descriptor Table, or GDT, is a table that tells the CPU about memory segments.
It is able to have up to 8192 entries, and each entry is a complex data structure.

### Local Descriptor Table
Like the GDT, it holds segment descriptors.
However, every task and thread can have their own LDT, so the OS can change the LDT register when the task switches.
This means that the LDT should not hold code for interrupt handlers, system-wide resources, or system segments like other LDTs.

The GDT is able to hold references to LDTs.

### Segment Selector
This is a 16-bit binary data structure that identifies a segment in either the GDT or the current LDT.
The six segmentation registers, `cs`, `ds`, `ss`, `es`, `fs`, and `gs`, hold segment selectors.


# Resources
[OSDev](https://wiki.osdev.org/Global_Descriptor_Table)
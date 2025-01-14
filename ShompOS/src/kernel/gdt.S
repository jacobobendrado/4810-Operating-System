# gdt.S
# Global Descriptor table implementation
# Cedarville University 2024-25 OSDev Team

# GDT - Global Descriptor Table
.section .data
gdt_start:
gdt_null:       # Entry 1: Null entry must be included first (error check)
    .long 0x0   # double word = 4 bytes = 32 bits
    .long 0x0
gdt_code:       # Entry 2: Code segment descriptor
    # Structure:
    # Segment Base Address (base) = 0x0
    # Segment Limit (limit) = 0xfffff
    .word 0xffff    # Limit bits 0-15
    .word 0x0000    # Base bits 0-15
    .byte 0x00      # Base bits 16-23
    # Flag Set 1:
        # Segment Present: 0b1
        # Descriptor Privilege level: 0x00 (ring 0)
        # Descriptor Type: 0b1 (code/data)
    # Flag Set 2: Type Field
        # Code: 0b1 (this is a code segment)
        # Conforming: 0b0 (Code w/ lower privilege may not call this)
        # Readable: 0b1 (Readable or execute only? Readable means we can read code constants)
        # Accessed: 0b0 (Used for debugging and virtual memory. CPU sets bit when accessing segment)
    .byte 0x9A      # Flag set 1 and 2 (10011010b)
    # Flag Set 3
        # Granularity: 0b1 (Set to 1 multiplies limit by 4K. Shift 0xfffff 3 bytes left, allowing to span full 32G of memory)
        # 32-bit default: 0b1
        # 64-bit segment: 0b0
        # AVL: 0b0
    .byte 0xCF      # Flag set 3 and limit bits 16-19 (11001111b)
    .byte 0x00      # Base bits 24-31
gdt_data:
    # Same except for code flag:
        # Code: 0b0
    .word 0xffff    # Limit bits 0-15
    .word 0x0000    # Base bits 0-15
    .byte 0x00      # Base bits 16-23
    .byte 0x92      # Flag set 1 and 2 (10010010b)
    .byte 0xCF      # 2nd flags and limit bits 16-19 (11001111b)
    .byte 0x00      # Base bits 24-31
gdt_end:        # Needed to calculate GDT size for inclusion in GDT descriptor

# GDT Descriptor
gdt_descriptor:
    .word gdt_end - gdt_start - 1    # Size of GDT, always less one
    .long gdt_start

# Define constants
.set CODE_SEG, gdt_code - gdt_start
.set DATA_SEG, gdt_data - gdt_start

# In protected mode, set DS = INDEX to select GDT entries
# Then CPU knows to use segment at that offset
# Example: (0x0: NULL segment; 0x8: CODE segment; 0x10: DATA segment)

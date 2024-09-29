### Purpose
>These notes compile key information about file systems, with a specific focus on their implementation in ShompOS, our educational operating system project. The primary aim is to provide a comprehensive reference for understanding and implementing a basic file system, particularly the ext2 file system, in the context of OS development.

#### Resources

| Title                                                       | Source                 | Link                                                                                   |
| ----------------------------------------------------------- | ---------------------- | -------------------------------------------------------------------------------------- |
| File Systems Wiki Page                                      | OSDev.org              | [Here](https://wiki.osdev.org/File_Systems)                                            |
| Ext2 Wiki Page                                              | OSDev.org              | [Here](https://wiki.osdev.org/Ext2)                                                    |
| How to Implement a Filesystem Forum Thread                  | OSDev.org              | [Here](https://f.osdev.org/viewtopic.php?t=28306&sid=c2dc0a3b977c0e14f50c20199a62e751) |
| Section of YouTube video on EXT file systems                | ExplainingComputers    | [Here](https://youtu.be/_h30HBYxtws?si=fEiR47AHrB0qZPKb&t=299)                         |
| Ext2 Wikipedia Page                                         | Wikipedia              | [Here](https://en.wikipedia.org/wiki/Ext2)                                             |
| Inode Pointer Structure Wikipedia Page                      | Wikipedia              | [Here](https://en.wikipedia.org/wiki/Inode_pointer_structure)                          |
| Design and Implementation of the Second Extended Filesystem | Card, Tso, and Tweedie | [Here](https://web.mit.edu/tytso/www/linux/ext2intro.html)                             |
| The Ext2 Filesystem Overview                                | Smith College          | [Here](https://www.science.smith.edu/~nhowe/262/oldlabs/ext2.html)                     |

### Inodes
- Also known as index nodes or information nodes
- Each file is made up of *data blocks* (the sectors that contain the raw data bits), *index blocks* (containing pointers to data blocks so that you know which sector is the nth in the sequence), and one *inode block*
- Inodes are an **alternative** to the File Allocation Table (FAT)
	- General rule of thumb: MS-DOS uses FAT, UNIX uses inodes


### Recommended File Systems for ShompOS
**[FAT](https://wiki.osdev.org/FAT "FAT")**

- `+` Can be read and written by virtually all OSes
- `+` The 'standard' for floppies
- `+` Relatively easy to implement
- `-` Part of it involving long filenames and compatibility is patented by Microsoft
- `-` Large overhead
- `-` No support for large (>4 GB) files
- `-` No support for Unix permissions

**[Ext2](https://wiki.osdev.org/Ext2 "Ext2")** (Preferred Choice for ShompOS)

- `+` Supports large files (with an extension)
- `+` Supports Unix permissions
- `+` Can be put on floppies
- `+` Can be read and written from Linux
- `-` Can not natively be read and written from Windows (but drivers are available)
- `-` Very large overhead
- `-` Of these beginner filesystems, this is the most complex




### Ext File System Family
-  In 1992, the "extended file system" (**ext**) was released for Linux
- In 1993, **ext2** was released
	- The default file system for Linux for many years
- In 2001, **ext3** was released
	- Introduced journaling to protect against corruption from crashes and power failures
- In 2008, **ext4** was released and became the default file system for Linux
	- Maximum file size of 16TB and volumes up to 1EB
- No native Windows or macOS support

## Ext2
### Ext File System Family
-  In 1992, the "extended file system" (**ext**) was released for Linux
- In 1993, **ext2** was released
	- The default file system for Linux for many years
- In 2001, **ext3** was released
	- Introduced journaling to protect against corruption from crashes and power failures
- In 2008, **ext4** was released and became the default file system for Linux
	- Maximum file size of 16TB and volumes up to 1EB
- No native Windows or macOS support

### Core Components
- The space in ext2 is split up into blocks. These blocks are grouped into block groups
- There are typically thousands of blocks of a large file system
- Each block contains a copy of the superblock and block group descriptor table, and all block groups contain a block bitmap, an inode bitmap, an inode table, and the actually data blocks
- The superblock contains important information that is crucial for booting the operating system. While there may be many copies of the superblock, typically only the first copy is used for booting the system. The first copy is typically found on the first block of the system
- The group descriptor stores the location of the first bitmap, inode bitmap, and start of the first inode table for every block group. These are stored in a group descriptor table
#### Inodes
- **Every file or directory is represented as an inode**
- The inode contains data about the size, permissions, ownership, and location on disk of the file or directory
- When a user requests an I/O operation on the file, the kernel code converts the current offset to a block number, uses this number as an index in the block addresses table and reads or writes the physical block.
- Like blocks, each inode has a numerical address. It is extremely important to note that unlike block addresses, **inode addresses start at 1**.
###### Inode Structure Diagram
>![[ext2-inode-structure.png]]

In the past, the structure may have consisted of about twelve pointers, but most modern file systems use fifteen pointers. These pointers consist of (assuming 15 pointers in the inode)

- 12 **direct pointers** that directly point to blocks of the file's data
- 1 **singly indirect pointer** (pointing to a block of direct pointers)
- 1 **doubly indirect pointer** (pointing to a block of single indirect pointers)
- 1 **triply indirect pointer** (pointing to a block of doubly indirect pointers)

The levels of indirection indicate the number of pointer that must be followed before reaching actual file data.

- This structure allows files to be easily allocated to non-contiguous block and data at particular locations in a file to be located easily
- The logical blocks are a fixed size, making it easy to traverse
- Unlike inodes, which are fixed in number and allocated in a special part of the file system, the indirect blocks may be of any number and are allocated in the same part of the file system as data blocks

###### Inode Data Structure

| Starting<br><br>Byte | Ending<br><br>Byte | Size<br><br>in Bytes | Field Description                                                                                                                                             |
| -------------------- | ------------------ | -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0                    | 1                  | 2                    | Type and Permissions ([see below](https://wiki.osdev.org/Ext2#Inode_Type_and_Permissions))                                                                    |
| 2                    | 3                  | 2                    | User ID                                                                                                                                                       |
| 4                    | 7                  | 4                    | Lower 32 bits of size in bytes                                                                                                                                |
| 8                    | 11                 | 4                    | Last Access Time (in [POSIX time](https://en.wikipedia.org/wiki/Unix_time))                                                                                   |
| 12                   | 15                 | 4                    | Creation Time (in [POSIX time](https://en.wikipedia.org/wiki/Unix_time))                                                                                      |
| 16                   | 19                 | 4                    | Last Modification time (in [POSIX time](https://en.wikipedia.org/wiki/Unix_time))                                                                             |
| 20                   | 23                 | 4                    | Deletion time (in [POSIX time](https://en.wikipedia.org/wiki/Unix_time))                                                                                      |
| 24                   | 25                 | 2                    | Group ID                                                                                                                                                      |
| 26                   | 27                 | 2                    | Count of hard links (directory entries) to this inode. When this reaches 0, the data blocks are marked as unallocated.                                        |
| 28                   | 31                 | 4                    | Count of disk sectors (not Ext2 blocks) in use by this inode, not counting the actual inode structure nor directory entries linking to the inode.             |
| 32                   | 35                 | 4                    | Flags ([see below](https://wiki.osdev.org/Ext2#Inode_Flags))                                                                                                  |
| 36                   | 39                 | 4                    | [Operating System Specific value #1](https://wiki.osdev.org/Ext2#OS_Specific_Value_1)                                                                         |
| 40                   | 43                 | 4                    | Direct Block Pointer 0                                                                                                                                        |
| 44                   | 47                 | 4                    | Direct Block Pointer 1                                                                                                                                        |
| 48                   | 51                 | 4                    | Direct Block Pointer 2                                                                                                                                        |
| 52                   | 55                 | 4                    | Direct Block Pointer 3                                                                                                                                        |
| 56                   | 59                 | 4                    | Direct Block Pointer 4                                                                                                                                        |
| 60                   | 63                 | 4                    | Direct Block Pointer 5                                                                                                                                        |
| 64                   | 67                 | 4                    | Direct Block Pointer 6                                                                                                                                        |
| 68                   | 71                 | 4                    | Direct Block Pointer 7                                                                                                                                        |
| 72                   | 75                 | 4                    | Direct Block Pointer 8                                                                                                                                        |
| 76                   | 79                 | 4                    | Direct Block Pointer 9                                                                                                                                        |
| 80                   | 83                 | 4                    | Direct Block Pointer 10                                                                                                                                       |
| 84                   | 87                 | 4                    | Direct Block Pointer 11                                                                                                                                       |
| 88                   | 91                 | 4                    | Singly Indirect Block Pointer (Points to a block that is a list of block pointers to data)                                                                    |
| 92                   | 95                 | 4                    | Doubly Indirect Block Pointer (Points to a block that is a list of block pointers to Singly Indirect Blocks)                                                  |
| 96                   | 99                 | 4                    | Triply Indirect Block Pointer (Points to a block that is a list of block pointers to Doubly Indirect Blocks)                                                  |
| 100                  | 103                | 4                    | Generation number (Primarily used for NFS)                                                                                                                    |
| 104                  | 107                | 4                    | In Ext2 version 0, this field is reserved. In version >= 1, Extended attribute block (File ACL).                                                              |
| 108                  | 111                | 4                    | In Ext2 version 0, this field is reserved. In version >= 1, Upper 32 bits of file size (if feature bit set) if it's a file, Directory ACL if it's a directory |
| 112                  | 115                | 4                    | Block address of fragment                                                                                                                                     |
| 116                  | 127                | 12                   | [Operating System Specific Value #2](https://wiki.osdev.org/Ext2#OS_Specific_Value_2)                                                                         |
To see this in action, run `stat <filename>` in a Linux terminal. `stat` displays information about the specified file(s).
#### Ext2 Physical Structure
The physical structure of a filesystem is represented in this table:

>![[ext2-structure-1.png]]

Each block group contains a redundant copy of crucial filesystem control informations (superblock and the filesystem descriptors) and also contains a part of the filesystem (a block bitmap, an inode bitmap, a piece of the inode table, and data blocks). The structure of a block group is represented in this table:

>![[ext2-structure-2.png]]

Using block groups is a big win in terms of reliability: since the control structures are replicated in each block group, it is easy to recover from a filesystem where the superblock has been corrupted. This structure also helps to get good performances: by reducing the distance between the inode table and the data blocks, it is possible to reduce the disk head seeks during I/O on files.

Here is another pictorial representation:
>![[ext2-structure-5.png]]
#### Directories
- Each directory is a list of directory entries
- Each directory entry associates one file name with one inode number, and consists of the inode number, the length of the file name, and the actual text of the file name
- The root directory is always stored in *inode number two*, so that the file system code can find it at mount time
- Subdirectories are implemented by storing the name of the subdirectory in the name field, and the inode number of the subdirectory in the inode field
- Hard links are implemented by storing the same inode number with more than one file name
- Accessing the file by either name results in the same inode number, and therefore the same data
- The special directories "." (current directory) and ".." (parent directory) are implemented by storing the names "." and ".." in the directory, and the inode number of the current and parent directories in the inode field. The only special treatment these two entries receive is that they are automatically created when any new directory is made, and they cannot be deleted.
###### Basic Directory Structure
>![[Pasted image 20240929173024.png]]

###### Ext2 Directory Physical Structure
In Ext2fs, directories are managed as linked lists of variable length entries. Each entry contains the inode number, the entry length, the file name and its length. By using variable length entries, it is possible to implement long file names without wasting disk space in directories. The structure of a directory entry is shown in this table:

>![[ext2-structure-3.png]]

As an example, The next table represents the structure of a directory containing three files: file1, long_file_name, and f2:
> ![[ext2-structure-4.png]]

#### Allocating Data
- When a new file or directory is created, ext2 must decide where to store the data. If the disk is mostly empty, then data can be stored almost anywhere. However, clustering the data with related data will minimize seek times and maximize performance.
- ext2 attempts to allocate each new directory in the group containing its parent directory, on the theory that accesses to parent and children directories are likely to be closely related
- ext2 also attempts to place files in the same group as their directory entries, because directory accesses often lead to file accesses. However, if the group is full, then the new file or new directory is placed in some other non-full group.

## The Virtual File System
- The Linux kernel contains a Virtual File System layer which is used during system calls acting on files
- The VFS is an indirection layer which handles the file oriented system calls and calls the necessary functions in the physical filesystem code to do the I/O
- When a process issues a file oriented system call, the kernel calls a function contained in the VFS. This function handles the structure independent manipulations and redirects the call to a function contained in the physical filesystem code, which is responsible for handling the structure dependent operations. Filesystem code uses the buffer cache functions to request I/O on devices
- **The VFS defines a set of functions that every filesystem has to implement**
- This interface is made up of a set of operations associated to three kinds of objects: filesystems, inodes, and open files
###### Virtual File System Diagram
> ![[VFS-Scheme 1.png]]

## Quick Summaries

### How To Read An Inode

1. Read the Superblock to find the size of each block, the number of blocks per group, number Inodes per group, and the starting block of the first group (Block Group Descriptor Table).
2. Determine which block group the inode belongs to.
3. Read the Block Group Descriptor corresponding to the Block Group which contains the inode to be looked up.
4. From the Block Group Descriptor, extract the location of the block group's inode table.
5. Determine the index of the inode in the inode table.
6. Index the inode table (taking into account non-standard inode size).

Directory entry information and file contents are located within the data blocks that the Inode points to.

### How To Read the Root Directory

The root directory's inode is defined to always be 2. Read/parse the contents of inode 2.

### Potential Ext2 Implemention Plan
The first step in implementing an Ext2 driver is to find, extract, and parse the superblock. The Superblock contains all information about the layout of the file system and possibly contains other important information like what optional features were used to create the file system. Once you have finished with the Superblock, the next step is to look at the [Block Group Descriptor Table](https://wiki.osdev.org/Ext2#Block_Group_Descriptor_Table)


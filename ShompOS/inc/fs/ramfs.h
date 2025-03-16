// ramfs.h
// header file for in memory file system
// Cedarville University 2024-25 OSDev Team

#ifndef RAMFS_H
#define RAMFS_H

// Global file descriptor table
#define MAX_FDS 64          // Maximum number of open files

// File descriptor definitions
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

// Add these if not already defined in your codebase
#ifndef _SSIZE_T_DEFINED
typedef long ssize_t;
#define _SSIZE_T_DEFINED
#endif

#ifndef _OFF_T_DEFINED
typedef long off_t;
#define _OFF_T_DEFINED
#endif

#include <stddef.h> // For size_t

// File structure
typedef struct ramfs_file {
    char *name;         // File name
    char *data;         // File contents
    size_t size;        // File size
} ramfs_file_t;

// Directory structure
typedef struct ramfs_dir {
    char *name;                  // Directory name
    struct ramfs_dir *parent;    // Parent directory
    struct ramfs_file **files;   // Array of files in this directory
    size_t file_count;           // Number of files
    struct ramfs_dir **subdirs;  // Array of subdirectories
    size_t subdir_count;         // Number of subdirectories
} ramfs_dir_t;

// File descriptor structure
typedef struct {
    int fd;                 // File descriptor number
    ramfs_file_t *file;     // Pointer to the file
    size_t position;        // Current position in the file
    int flags;              // Open flags (read, write, etc.)
} ramfs_fd_t;

extern ramfs_fd_t *fd_table[MAX_FDS];
extern int fd_count;

// Function prototypes for RAMFS operations
ramfs_dir_t *ramfs_create_root();
ramfs_file_t *ramfs_create_file(ramfs_dir_t *dir, const char *name, const char *data, size_t size);
void ramfs_delete_file(ramfs_dir_t *dir, const char *name);
ramfs_dir_t *ramfs_create_dir(ramfs_dir_t *parent, const char *name);
ramfs_dir_t *ramfs_find_dir(ramfs_dir_t *root, const char *path);

int ramfs_init_fd_system(void);
int ramfs_open(ramfs_dir_t *root, const char *path, int flags);
ssize_t ramfs_read(int fd, void *buf, size_t count);
ssize_t ramfs_write(int fd, const void *buf, size_t count);
off_t ramfs_seek(int fd, off_t offset, int whence);
int ramfs_close(int fd);
void test_fd_system(ramfs_dir_t *root);


#endif // RAMFS_H

// ramfs.h
// header file for in memory file systepm
// Cedarville University 2024-25 OSDev Team

#ifndef RAMFS_H
#define RAMFS_H

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define MAX_FD 64  // Maximum number of file descriptors

// File mode flags
#define O_RDONLY 0x0001
#define O_WRONLY 0x0002
#define O_RDWR   0x0003
#define O_APPEND 0x0008
#define O_CREAT  0x0100

// File descriptor structure
typedef struct {
    ramfs_file_t *file;     // Pointer to the file
    ramfs_dir_t *dir;       // Directory containing the file
    size_t position;        // Current read/write position
    int flags;              // Open flags (read/write/append)
    int is_special;         // Flag for special files like stdin/stdout/stderr
} ramfs_fd_t;

// Global file descriptor table
extern ramfs_fd_t *fd_table[MAX_FD];

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
    size_t file_count;           // Number of fileps
    struct ramfs_dir **subdirs;  // Array of subdirectories
    size_t subdir_count;         // Number of subdirectories
} ramfs_dir_t;

// Function prototypes for RAMFS operations
ramfs_dir_t *ramfs_create_root();
ramfs_file_t *ramfs_create_file(ramfs_dir_t *dir, const char *name, const char *data, size_t size);
void ramfs_delete_file(ramfs_dir_t *dir, const char *name);
ramfs_dir_t *ramfs_create_dir(ramfs_dir_t *parent, const char *name);
ramfs_dir_t *ramfs_find_dir(ramfs_dir_t *root, const char *path);

// File descriptor functions
int ramfs_open(ramfs_dir_t *root, const char *path, int flags);
int ramfs_close(int fd);
ssize_t ramfs_read(int fd, void *buf, size_t count);
ssize_t ramfs_write(int fd, const void *buf, size_t count);
off_t ramfs_lseek(int fd, off_t offset, int whence);
int ramfs_init_std_descriptors(ramfs_dir_t *root);
#include <stddef.h> // For size_t
#endif // RAMFS_H

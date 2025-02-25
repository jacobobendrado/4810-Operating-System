// ramfs.h
// header file for in memory file system
// Cedarville University 2024-25 OSDev Team

#ifndef RAMFS_H
#define RAMFS_H

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

// Function prototypes for RAMFS operations
ramfs_dir_t *ramfs_create_root();
ramfs_file_t *ramfs_create_file(ramfs_dir_t *dir, const char *name, const char *data, size_t size);
void ramfs_delete_file(ramfs_dir_t *dir, const char *name);
ramfs_dir_t *ramfs_create_dir(ramfs_dir_t *parent, const char *name);
ramfs_dir_t *ramfs_find_dir(ramfs_dir_t *root, const char *path);
ramfs_dir_t *init_fs();
int init_mnt(ramfs_dir_t *mnt);

#endif // RAMFS_H

// ramfs.c
// simple implementation of an in memory filesystem
// Cedarville University 2024-25 OSDev Team

#include <heap.h>       // For allocate, free
#include <fake_libc.h>
#include <ramfs.h>
#include <string.h>

// create the root directory
ramfs_dir_t *ramfs_create_root() {
    ramfs_dir_t *root = allocate(sizeof(ramfs_dir_t));
    if (!root) return NULL;
    root->name = strdup("/");
    root->parent = NULL;
    root->files = NULL;
    root->file_count = 0;
    root->subdirs = NULL;
    root->subdir_count = 0;
    return root;
}

// create a directory
ramfs_dir_t *ramfs_create_dir(ramfs_dir_t *parent, const char *name) {
    if (!parent || !name) return NULL;

    // Allocate new directory structure
    ramfs_dir_t *new_dir = (ramfs_dir_t *)allocate(sizeof(ramfs_dir_t));
    if (!new_dir) return NULL;

    // Initialize the directory
    new_dir->name = strdup(name);  // or directly copy if you don't have strdup
    new_dir->parent = parent;
    new_dir->files = NULL;
    new_dir->file_count = 0;
    new_dir->subdirs = NULL;
    new_dir->subdir_count = 0;

    // Expand parent's subdirs array
    ramfs_dir_t **new_subdirs = allocate((parent->subdir_count + 1) * sizeof(ramfs_dir_t*));
    if (!new_subdirs) {
        void *dir_ptr = new_dir;
        free(&dir_ptr);
        return NULL;
    }

    // Copy existing subdirs if any
    if (parent->subdirs) {
        memcpy(new_subdirs, parent->subdirs, parent->subdir_count * sizeof(ramfs_dir_t*));
        void *old_subdirs = parent->subdirs;
        free(&old_subdirs);
    }

    // Add new directory
    parent->subdirs = new_subdirs;
    parent->subdirs[parent->subdir_count++] = new_dir;

    return new_dir;
}

// create a file
ramfs_file_t *ramfs_create_file(ramfs_dir_t *dir, const char *name, const char *data, size_t size) {
    if (!dir || !name || !data) return NULL;

    // Allocate new file structure
    ramfs_file_t *new_file = (ramfs_file_t *)allocate(sizeof(ramfs_file_t));
    if (!new_file) return NULL;

    // Allocate and copy the name
    new_file->name = strdup(name);
    if (!new_file->name) {
        void *file_ptr = new_file;
        free(&file_ptr);
        return NULL;
    }

    // Allocate and copy the data
    new_file->data = allocate(size);
    if (!new_file->data) {
        void *name_ptr = new_file->name;
        void *file_ptr = new_file;
        free(&name_ptr);
        free(&file_ptr);
        return NULL;
    }
    memcpy(new_file->data, data, size);
    new_file->size = size;

    // Expand directory's files array
    ramfs_file_t **new_files = allocate((dir->file_count + 1) * sizeof(ramfs_file_t*));
    if (!new_files) {
        void *name_ptr = new_file->name;
        void *data_ptr = new_file->data;
        void *file_ptr = new_file;
        free(&name_ptr);
        free(&data_ptr);
        free(&file_ptr);
        return NULL;
    }

    // Copy existing files if any
    if (dir->files) {
        memcpy(new_files, dir->files, dir->file_count * sizeof(ramfs_file_t*));
        void *old_files = dir->files;
        free(&old_files);
    }

    // Add new file
    dir->files = new_files;
    dir->files[dir->file_count++] = new_file;

    return new_file;
}

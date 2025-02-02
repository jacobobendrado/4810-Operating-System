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

// Delete a file from a directory
void ramfs_delete_file(ramfs_dir_t *dir, const char *name) {
    if (!dir || !name || !dir->files || dir->file_count == 0) return;

    // Find the file index
    size_t file_idx = (size_t)-1;
    for (size_t i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, name) == 0) {
            file_idx = i;
            break;
        }
    }

    // If file not found, return
    if (file_idx == (size_t)-1) return;

    // Free the file's resources
    void *name_ptr = dir->files[file_idx]->name;
    void *data_ptr = dir->files[file_idx]->data;
    void *file_ptr = dir->files[file_idx];
    free(&name_ptr);
    free(&data_ptr);
    free(&file_ptr);

    // If it's not the last file, shift remaining files left
    if (file_idx < dir->file_count - 1) {
        memmove(&dir->files[file_idx],
                &dir->files[file_idx + 1],
                (dir->file_count - file_idx - 1) * sizeof(ramfs_file_t*));
    }

    // Shrink the files array if this wasn't the only file
    if (dir->file_count > 1) {
        ramfs_file_t **new_files = allocate((dir->file_count - 1) * sizeof(ramfs_file_t*));
        if (new_files) {
            memcpy(new_files, dir->files, (dir->file_count - 1) * sizeof(ramfs_file_t*));
            void *old_files = dir->files;
            free(&old_files);
            dir->files = new_files;
        }
    } else {
        // If it was the only file, just free the array
        void *files_ptr = dir->files;
        free(&files_ptr);
        dir->files = NULL;
    }

    dir->file_count--;
}

// Find a directory given a path
ramfs_dir_t *ramfs_find_dir(ramfs_dir_t *root, const char *path) {
    if (!root || !path) return NULL;

    // Handle root directory case
    if (strcmp(path, "/") == 0) return root;

    // Make a copy of path that we can modify
    char *path_copy = strdup(path);
    if (!path_copy) return NULL;

    // Start at root
    ramfs_dir_t *current = root;

    // Skip leading slash if present
    char *token = strtok(path_copy, "/");

    // Traverse the path
    while (token) {
        // Look for directory with matching name
        ramfs_dir_t *found = NULL;
        for (size_t i = 0; i < current->subdir_count; i++) {
            if (strcmp(current->subdirs[i]->name, token) == 0) {
                found = current->subdirs[i];
                break;
            }
        }

        // If directory not found, clean up and return NULL
        if (!found) {
            void *path_ptr = path_copy;
            free(&path_ptr);
            return NULL;
        }

        current = found;
        token = strtok(NULL, "/");
    }

    void *path_ptr = path_copy;
    free(&path_ptr);
    return current;
}

// ramfs_executables.c
// functions that act as executables for the ramfs
// Cedarville University 2024-25 OSDev Team

#include <heap.h>       // For allocate, free
#include <fake_libc.h>
#include <ramfs.h>
#include <string.h>
#include <ramfs_executables.h>

void ramfs_pwd(ramfs_dir_t *dir) {
    if (!dir) return;

    // Find the length of the full path
    size_t length = 0;
    ramfs_dir_t *temp = dir;
    while (temp) {
        length += strlen(temp->name) + 1; // +1 for '/'
        temp = temp->parent;
    }

    // Build the path string
    char *path = (char *)allocate(length + 1);
    if (!path) return;
    path[length] = '\0';

    temp = dir;
    char *ptr = path + length;
    while (temp) {
        size_t name_len = strlen(temp->name);
        ptr -= name_len;
        memcpy(ptr, temp->name, name_len);
        if (temp->parent) {
            *(--ptr) = '/';
        }
        temp = temp->parent;
    }

    terminal_writestring(path);
    terminal_writestring("\n");
    void *path_ptr = path;
    free(&path_ptr);
}

void ramfs_mkdir(ramfs_dir_t *dir, const char *dirname) {
    if (!dir || !dirname) return;

    // Check if directory already exists
    for (size_t i = 0; i < dir->subdir_count; i++) {
        if (strcmp(dir->subdirs[i]->name, dirname) == 0) {
            terminal_writestring("Directory already exists: ");
            terminal_writestring(dirname);
            terminal_writestring("\n");
            return;
        }
    }

    // Create directory
    ramfs_dir_t *newdir = ramfs_create_dir(dir, dirname);

    if (newdir) {
        terminal_writestring("Created directory: ");
        terminal_writestring(dirname);
        terminal_writestring("\n");
    } else {
        terminal_writestring("Failed to create directory: ");
        terminal_writestring(dirname);
        terminal_writestring("\n");
    }
}

void ramfs_rm(ramfs_dir_t *dir, const char *filename) {
    if (!dir || !filename) return;

    // Find the file
    bool found = false;
    for (size_t i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, filename) == 0) {
            found = true;
            break;
        }
    }

    if (!found) {
        terminal_writestring("File not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return;
    }

    ramfs_delete_file(dir, filename);
    terminal_writestring("Removed file: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
}

void ramfs_ls(ramfs_dir_t *dir) {
    if (!dir) return;

    terminal_writestring("Contents of ");
    ramfs_pwd(dir);

    // List directories first
    for (size_t i = 0; i < dir->subdir_count; i++) {
        terminal_writestring("[DIR]  ");
        terminal_writestring(dir->subdirs[i]->name);
        terminal_writestring("\n");
    }

    // Then list files
    for (size_t i = 0; i < dir->file_count; i++) {
        terminal_writestring("[FILE] ");
        terminal_writestring(dir->files[i]->name);
        terminal_writestring("\n");
    }
}

void ramfs_cat(ramfs_dir_t *dir, const char *filename) {
    if (!dir || !filename) return;

    // Trim leading spaces
    while (*filename == ' ') filename++;

    // Find the file
    ramfs_file_t* file = NULL;
    for (size_t i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, filename) == 0) {
            file = dir->files[i];
            break;
        }
    }

    if (file) {
        terminal_writestring(file->data);
        terminal_writestring("\n");
    } else {
        terminal_writestring("File not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
    }
}

void ramfs_touch(ramfs_dir_t *dir, const char *filename) {
    if (!dir || !filename) return;

    // Trim leading spaces
    while (*filename == ' ') filename++;

    // Check for empty filename
    if (strlen(filename) == 0) {
        terminal_writestring("Usage: touch <filename>\n");
        return;
    }

    // Check if file already exists
    for (size_t i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, filename) == 0) {
            terminal_writestring("File already exists\n");
            return;
        }
    }

    ramfs_file_t* file = ramfs_create_file(dir, filename, "", 1);
    if (file) {
        terminal_writestring("Created file: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
    } else {
        terminal_writestring("Failed to create file\n");
    }
}

ramfs_dir_t *ramfs_cd(ramfs_dir_t *root, const char *dir_name) {
    return ramfs_find_dir(root, dir_name);
}

void ramfs_run(ramfs_dir_t *dir, const char *filename) {
    if (!dir || !filename) return;

    // Trim leading spaces
    while (*filename == ' ') filename++;

    // Find the file
    ramfs_file_t* file = NULL;
    for (size_t i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, filename) == 0) {
            file = dir->files[i];
            break;
        }
    }

    if (file) {
        init_elf(file, 0);
    } else {
        terminal_writestring("File not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
    }
}
// ramfs_executables.c
// functions that act as executables for the ramfs
// Cedarville University 2024-25 OSDev Team

#include <heap.h>       // For allocate(), free()
#include <kernel.h>     // For terminal_writestring()
#include <fake_libc.h>
#include <ramfs.h>
#include <string.h>
#include <ramfs_executables.h>
#include <kernel.h>
#include <elf.h>


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

    char* str = "The current directory is: {";
    ramfs_write(STDOUT_FILENO, str, strlen(str));
    ramfs_write(STDOUT_FILENO, path, strlen(path));
    str = "}\n";
    ramfs_write(STDOUT_FILENO, str, strlen(str));

    free(path);
}

void ramfs_mkdir(ramfs_dir_t *dir, const char *dirname) {
    if (!dir || !dirname) return;

    // Check if directory already exists
    for (size_t i = 0; i < dir->subdir_count; i++) {
        if (strcmp(dir->subdirs[i]->name, dirname) == 0) {
            char* str = "Directory already exists: ";
            ramfs_write(STDOUT_FILENO, str, strlen(str));
            ramfs_write(STDOUT_FILENO, dirname, strlen(dirname));
            ramfs_write(STDOUT_FILENO, "\n", 1);
            return;
        }
    }

    // Create directory
    ramfs_dir_t *newdir = ramfs_create_dir(dir, dirname);

    if (newdir) {
        char* str = "Created directory: ";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        ramfs_write(STDOUT_FILENO, dirname, strlen(dirname));
        ramfs_write(STDOUT_FILENO, "\n", 1);
    } else {
        char* str = "Failed to create directory: ";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        ramfs_write(STDOUT_FILENO, dirname, strlen(dirname));
        ramfs_write(STDOUT_FILENO, "\n", 1);
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
        char* str = "File not found: ";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        ramfs_write(STDOUT_FILENO, filename, strlen(filename));
        ramfs_write(STDOUT_FILENO, "\n", 1);
        return;
    }

    ramfs_delete_file(dir, filename);
    char* str = "Removed file: ";
    ramfs_write(STDOUT_FILENO, str, strlen(str));
    ramfs_write(STDOUT_FILENO, filename, strlen(filename));
    ramfs_write(STDOUT_FILENO, "\n", 1);
}

void ramfs_ls(ramfs_dir_t *dir) {
    if (!dir) return;

    char* str = "Contents of ";
    ramfs_write(STDOUT_FILENO, str, strlen(str));
    ramfs_pwd(dir);

    // List directories first
    for (size_t i = 0; i < dir->subdir_count; i++) {
        char* str = "[DIR]  ";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        ramfs_write(STDOUT_FILENO, dir->subdirs[i]->name, strlen(dir->subdirs[i]->name));
        ramfs_write(STDOUT_FILENO, "\n", 1);
    }

    // Then list files
    for (size_t i = 0; i < dir->file_count; i++) {
        char* str = "[FILE] ";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        ramfs_write(STDOUT_FILENO, dir->files[i]->name, strlen(dir->files[i]->name));
        str = "    size:";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        // TODO: itos() the size lol
        char str_buf[16] = {0};
        itos(str_buf, dir->files[i]->size);
        ramfs_write(STDOUT_FILENO, str_buf, strlen(str_buf));
        ramfs_write(STDOUT_FILENO, "\n", 1);
        
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
        ramfs_write(STDOUT_FILENO, file->data, strlen(file->data));
        ramfs_write(STDOUT_FILENO, "\n", 1);
    } else {
        char* str = "File not found: ";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        ramfs_write(STDOUT_FILENO, filename, strlen(filename));
        ramfs_write(STDOUT_FILENO, "\n", 1);
    }
}

void ramfs_touch(ramfs_dir_t *dir, const char *filename) {
    if (!dir || !filename) return;

    // Trim leading spaces
    while (*filename == ' ') filename++;

    // Check for empty filename
    if (strlen(filename) == 0) {
        char* str = "Usage: touch <filename>\n";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        return;
    }

    // Check if file already exists
    for (size_t i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, filename) == 0) {
            char* str = "File already exists\n";
            ramfs_write(STDOUT_FILENO, str, strlen(str));
            return;
        }
    }

    ramfs_file_t* file = ramfs_create_file(dir, filename, "", 1);
    if (file) {
        char* str = "Created file: ";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        ramfs_write(STDOUT_FILENO, filename, strlen(filename));
        ramfs_write(STDOUT_FILENO, "\n", 1);
    } else {
        char* str = "Failed to create file\n";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        ramfs_write(STDOUT_FILENO, "\n", 1);
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
        init_elf(file);
    } else {
        char* str = "File not found: ";
        ramfs_write(STDOUT_FILENO, str, strlen(str));
        ramfs_write(STDOUT_FILENO, filename, strlen(filename));
        ramfs_write(STDOUT_FILENO, "\n", 1);
    }
}

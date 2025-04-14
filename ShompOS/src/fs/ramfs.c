// ramfs.c
// Simple implementation of an in memory filesystem
// Cedarville University 2024-25 OSDev Team

#include <heap.h>       // For allocate, free
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
    new_dir->name = strdup(name);
    new_dir->parent = parent;
    new_dir->files = NULL;
    new_dir->file_count = 0;
    new_dir->subdirs = NULL;
    new_dir->subdir_count = 0;

    // Expand parent's subdirs array
    ramfs_dir_t **new_subdirs = allocate((parent->subdir_count + 1) * sizeof(ramfs_dir_t*));
    if (!new_subdirs) {
        void *dir_ptr = new_dir;
        free(dir_ptr);
        return NULL;
    }

    // Copy existing subdirs if any
    if (parent->subdirs) {
        memcpy(new_subdirs, parent->subdirs, parent->subdir_count * sizeof(ramfs_dir_t*));
        void *old_subdirs = parent->subdirs;
        free(old_subdirs);
    }

    // Add new directory
    parent->subdirs = new_subdirs;
    parent->subdirs[parent->subdir_count++] = new_dir;

    return new_dir;
}

// create a file
ramfs_file_t *ramfs_create_file(ramfs_dir_t *dir, const char *name, const char *data, size_t size) {
    if (!dir || !name || !data) return NULL;

    ramfs_file_t *new_file = (ramfs_file_t *)allocate(sizeof(ramfs_file_t));
    if (!new_file) return NULL;

    new_file->name = strdup(name);
    if (!new_file->name) {
        free(new_file);
        return NULL;
    }

    new_file->data = (size == 0) ? allocate(1) : allocate(size);
    if (!new_file->data) {
        free(new_file->name);
        free(new_file);
        return NULL;
    }

    memcpy(new_file->data, data, size);
    new_file->size = size;

    ramfs_file_t **new_files = allocate((dir->file_count + 1) * sizeof(ramfs_file_t*));
    if (!new_files) {
        free(new_file->data);
        free(new_file->name);
        free(new_file);
        return NULL;
    }

    if (dir->files) {
        memcpy(new_files, dir->files, dir->file_count * sizeof(ramfs_file_t*));
        free(dir->files);
    }

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
    free(name_ptr);
    free(data_ptr);
    free(file_ptr);

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
            free(old_files);
            dir->files = new_files;
        }
    } else {
        // If it was the only file, just free the array
        void *files_ptr = dir->files;
        free(files_ptr);
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
            free(path_ptr);
            return NULL;
        }

        current = found;
        token = strtok(NULL, "/");
    }

    void *path_ptr = path_copy;
    free(path_ptr);
    return current;
}

// Initialize the filesystem. Returns the root directory
ramfs_dir_t* init_fs() {

    ramfs_dir_t* root = ramfs_create_root();
    if (!root) {
        return NULL;
    }

    if (init_mnt(root)) {
        return NULL;
    }

    return root;
}

// Global file descriptor table
ramfs_fd_t *fd_table[MAX_FDS];
int fd_count = 0;

// Initialize the file descriptor system
int ramfs_init_fd_system(void) {
    // Initialize fd_table to NULL
    for (int i = 0; i < MAX_FDS; i++) {
        fd_table[i] = NULL;
    }
    fd_count = 0;
    return 0;
}

int ramfs_open(ramfs_dir_t *root, const char *path, int flags) {
    if (!root || !path || *path == '\0') {
        return -1;
    }

    // Copy path to safely manipulate it
    char *path_copy = strdup(path);
    if (!path_copy) {
        return -1;
    }

    // Extract filename and directory path
    char *last_slash = strchr(path_copy, '/');
    ramfs_dir_t *dir = root;
    char *filename = path_copy;

    if (last_slash) {
        *last_slash = '\0';  // Split into directory and filename
        filename = last_slash + 1;

        if (strlen(path_copy) > 0) {
            dir = ramfs_find_dir(root, path_copy);
            if (!dir) {
                free(path_copy);
                return -1;
            }
        }
    }

    // Locate the file in the directory
    ramfs_file_t *file = NULL;
    for (size_t i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, filename) == 0) {
            file = dir->files[i];
            break;
        }
    }

    if (!file) {
        free(path_copy);
        return -1;
    }

    // Find an available file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_FDS; i++) {
        if (fd_table[i] == NULL) {
            fd = i;
            break;
        }
    }

    if (fd == -1) {
        free(path_copy);
        return -1;
    }

    // Allocate file descriptor entry
    ramfs_fd_t *fd_entry = allocate(sizeof(ramfs_fd_t));
    if (!fd_entry) {
        free(path_copy);
        return -1;
    }

    fd_entry->fd = fd;
    fd_entry->file = file;
    fd_entry->position = 0;
    fd_entry->flags = flags;

    fd_table[fd] = fd_entry;
    fd_count++;

    free(path_copy);
    return fd;
}


// Read from a file using a file descriptor
ssize_t ramfs_read(int fd, void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FDS || fd_table[fd] == NULL) {
        return -1; // Invalid file descriptor
    }

    ramfs_fd_t *fd_entry = fd_table[fd];
    if (!(fd_entry->flags & O_RDONLY) && !(fd_entry->flags & O_RDWR)) {
        return -1; // Not readable
    }

    if (fd_entry->file == NULL) {
        return -1; // No file associated
    }

    size_t available = fd_entry->file->size - fd_entry->position;
    size_t bytes_to_read = (count > available) ? available : count;
    memcpy((char*)buf, fd_entry->file->data + fd_entry->position, bytes_to_read);
    fd_entry->position += bytes_to_read;
    return bytes_to_read;
}


// Write to a file using a file descriptor
ssize_t ramfs_write(int fd, const void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FDS || fd_table[fd] == NULL) {
        return -1; // Invalid file descriptor
    }

    ramfs_fd_t *fd_entry = fd_table[fd];

    if (!(fd_entry->flags & (O_WRONLY | O_RDWR))) {
        return -1; // Not writable
    }

    if (fd_entry->file == NULL) {
        return -1; // No file associated
    }

    // Move position to end if O_APPEND is set
    // TODO: Should be in open (consider using seek for file initially appending)
    if (fd_entry->flags & O_APPEND) {
        fd_entry->position = fd_entry->file->size;
    }

    size_t new_size = fd_entry->position + count;

    // TODO: Let the file shrink
    if (new_size > fd_entry->file->size) {
        // Resize file data buffer (manual realloc)
        char *new_data = allocate(new_size);
        if (!new_data) return -1;

        // Copy old data into new buffer
        if (fd_entry->file->data) {
            memcpy(new_data, fd_entry->file->data, fd_entry->file->size);
            void *old_data = fd_entry->file->data;
            free(old_data);
        }

        fd_entry->file->data = new_data;
        fd_entry->file->size = new_size;
    }

    // Write new data
    memcpy(fd_entry->file->data + fd_entry->position, buf, count);
    fd_entry->position += count;

    return count;
}



// TODO: Does this expand the file if needed? Or at least check for upper bounds?
// Change the position in a file
off_t ramfs_seek(int fd, off_t offset, int origin) {
    // Validate inputs
    if (fd < 0 || fd >= MAX_FDS || !fd_table[fd]) return -1;

    ramfs_fd_t *fd_entry = fd_table[fd];
    ramfs_file_t *file = fd_entry->file;

    // Calculate new position based on origin
    off_t new_position = 0;

    switch (origin) {
        case SEEK_SET: // From beginning of file
            new_position = offset;
            break;
        case SEEK_CUR: // From current position
            new_position = fd_entry->position + offset;
            break;
        case SEEK_END: // From end of file
            new_position = file->size + offset;
            break;
        default:
            return -1;
    }

    // Check if new position is valid
    if (new_position < 0) return -1;

    // Update position
    fd_entry->position = new_position;

    return new_position;
}

// Close a file descriptor
int ramfs_close(int fd) {
    if (fd < 0 || fd >= MAX_FDS || fd_table[fd] == NULL) {
        return -1; // Invalid fd
    }

    ramfs_fd_t *fd_entry = fd_table[fd];
    free(fd_entry);

    fd_table[fd] = NULL;
    fd_count--;

    return 0;
}


int init_stdio(ramfs_dir_t *root) {
    // Create special files for stdin, stdout, stderr with size 0
    // ramfs_file_t *stdin_file = ramfs_create_file(root, "stdin", "", 0);
    // ramfs_file_t *stdout_file = ramfs_create_file(root, "stdout", "", 0);
    // ramfs_file_t *stderr_file = ramfs_create_file(root, "stderr", "", 0);
    // if (!stdin_file || !stdout_file || !stderr_file)
        // return -1;
    
    //ramfs_open(root, "stdin", O_RDWR | O_APPEND);
    //ramfs_open(root, "stdout", O_WRONLY);
    //ramfs_open(root, "stderr", O_WRONLY);
    

    char* filename = "stdstream";
    
    ramfs_file_t *stdstream_file = ramfs_create_file(root, filename, "", 1);
    if (!stdstream_file) return -1;

    // stdin
    ramfs_open(root, filename, O_RDWR | O_APPEND);
    // stdout
    ramfs_open(root, filename, O_WRONLY);

    return 0;
}

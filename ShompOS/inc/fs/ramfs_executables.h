// ramfs_executables.h
// header file for in memory file system "executables"
// Cedarville University 2024-25 OSDev Team

#ifndef RAMFS_EXECUTABLES_H
#define RAMFS_EXECUTABLES_H

#include <stddef.h> // For size_t

void ramfs_ls(ramfs_dir_t *dir);
void ramfs_pwd(ramfs_dir_t *dir);
void ramfs_cat(ramfs_dir_t *dir, const char *filename);
void ramfs_touch(ramfs_dir_t *dir, const char *filename);
void ramfs_mkdir(ramfs_dir_t *dir, const char *dirname);
void ramfs_rm(ramfs_dir_t *dir, const char *filename);
ramfs_dir_t *ramfs_cd(ramfs_dir_t *root, const char *filename);


#endif // RAMFS_EXECUTABLES_H

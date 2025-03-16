#ifndef STRING_H
#define STRING_H

#include <stddef.h>  // For size_t

// Basic string manipulation functions
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
char *strdup(const char *s);
int strncmp(const char *s1, const char *s2, size_t n);
char *strtok(char *str, const char *delim);
char *strchr(const char *s, int c);
void *memset(void *ptr, int value, size_t num);


#endif

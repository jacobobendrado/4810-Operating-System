#include <string.h>
#include <heap.h>           //

// strlen: Return length of string
size_t strlen(const char *s) {
    size_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

// strcmp: Compare two strings
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// strcpy: Copy string from src to dest
char *strcpy(char *dest, const char *src) {
    char *dest_ptr = dest;
    while ((*dest_ptr++ = *src++)) {
        // copy character by character until null terminator
    }
    return dest;
}

// memcpy: Copy n bytes from src to dest
void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

// memmove: Move n bytes from src to dest (handles overlap)
void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d < s || (d >= s + n)) { // No overlap
        while (n--) {
            *d++ = *s++;
        }
    } else { // Overlap, copy backwards
        d += n;
        s += n;
        while (n--) {
            *(--d) = *(--s);
        }
    }
    return dest;
}

char *strdup(const char *s) {
    size_t len = 0;

    // Find the length of the string
    while (s[len]) {
        len++;
    }

    // Allocate memory for the new string (+1 for the null terminator)
    char *copy = allocate(len + 1);
    if (!copy) {
        return NULL; // Memory allocation failed
    }

    // Copy the string into the new allocated memory
    for (size_t i = 0; i < len; i++) {
        copy[i] = s[i];
    }

    // Null-terminate the new string
    copy[len] = '\0';

    return copy;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) return 0;

    while (n-- > 0 && *s1 && *s2) {
        if (*s1 != *s2) {
            return (*(unsigned char *)s1 - *(unsigned char *)s2);
        }
        s1++;
        s2++;
    }

    if (n > 0) {
        if (*s1) return 1;
        if (*s2) return -1;
    }

    return 0;
}

char *strtok(char *str, const char *delim) {
    static char *last;

    if (str) {
        last = str;
    } else if (!last) {
        return NULL;
    }

    // Skip leading delimiters
    while (*last && strchr(delim, *last)) {
        last++;
    }

    // No more tokens
    if (!*last) {
        last = NULL;
        return NULL;
    }

    // Find end of token
    char *token = last;
    while (*last && !strchr(delim, *last)) {
        last++;
    }

    if (*last) {
        *last++ = '\0';
    } else {
        last = NULL;
    }

    return token;
}


char *strchr(const char *s, int c) {
    while (*s != '\0') {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }
    if ((char)c == '\0') {
        return (char *)s;
    }
    return NULL;
}

void *memset(void *ptr, int value, size_t num) {
    unsigned char *p = ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// generic struct for any linked lists. 8 bytes.
typedef struct _list_header {
    struct _list_header* next;
    struct _list_header* prev;
} list_header;

bool is_end_of_list(list_header* node);
bool is_head_of_list(list_header* node);
size_t strlen(const char* str);
int max(int a, int b);

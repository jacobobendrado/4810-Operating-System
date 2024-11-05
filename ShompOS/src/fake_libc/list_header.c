#include <fake_libc/list_header.h>
#include <stdbool.h>

inline bool is_end_of_list(list_header* node) {
    return node->next == node;
}

inline bool is_head_of_list(list_header* node) {
    return node->prev == node;
}
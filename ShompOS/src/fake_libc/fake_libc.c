#include <stddef.h>
#include <stdint.h>
#include <fake_libc.h>

inline bool is_end_of_list(list_header* node) {
    return node->next == node;
}

inline bool is_head_of_list(list_header* node) {
    return node->prev == node;
}

inline int max(int a, int b) {
	return a >= b ? a : b;
}

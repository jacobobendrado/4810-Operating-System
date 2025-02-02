#include <stddef.h>
#include <stdint.h>
#include <fake_libc/fake_libc.h>

inline bool is_end_of_list(list_header* node) {
    return node->next == node;
}

inline bool is_head_of_list(list_header* node) {
    return node->prev == node;
}

size_t strlen(const char* str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

inline int max(int a, int b) {
	return a >= b ? a : b;
}
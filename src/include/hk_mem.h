#pragma once

#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
    size_t size;
    char *ptr;
} hk_mem;

hk_mem *hk_mem_create();
void hk_mem_append(hk_mem *mem, hk_mem *mem_to_append);
void hk_mem_append_buffer(hk_mem *mem, void *data, size_t size);
void hk_mem_append_string(hk_mem *mem, const char *string);
void hk_mem_append_string_terminator(hk_mem *mem);
void hk_mem_prepend_buffer(hk_mem *mem, void *data, size_t size);
void hk_mem_set(hk_mem *mem, size_t size);
void hk_mem_free(hk_mem *mem);

char *hk_mem_get_str(hk_mem *mem);
bool hk_mem_cmp(hk_mem *mem1, hk_mem *mem2);
bool hk_mem_cmp_str(hk_mem *mem, const char* str);
void hk_mem_log(const char *title, hk_mem *mem);

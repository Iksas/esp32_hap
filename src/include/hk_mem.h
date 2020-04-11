#pragma once

#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
    size_t size;
    char *ptr;
} hk_mem;

#define HK_MEM_ASSIGN_BYTE_STR(var_name, mem)               \
    char var_name[mem->size > 0 ? mem->size * 2 : 1];       \
    if (mem->size > 0)                                      \
    {                                                       \
        for (size_t i = 0; i < mem->size; i++)              \
        {                                                   \
            sprintf(var_name + i * 2, "%02x", mem->ptr[i]); \
        };                                                  \
    }                                                       \
    else                                                    \
    {                                                       \
        sprintf(var_name, "-");                             \
    }

hk_mem *hk_mem_init();
void hk_mem_append(hk_mem *mem, hk_mem *mem_to_append);
void hk_mem_append_buffer(hk_mem *mem, void *data, size_t size);
void hk_mem_append_string(hk_mem *mem, const char *string);
void hk_mem_append_string_terminator(hk_mem *mem);
void hk_mem_insert_buffer(hk_mem *mem, void *data, size_t size, size_t index);
void hk_mem_set(hk_mem *mem, size_t size);
void hk_mem_free(hk_mem *mem);

bool hk_mem_equal(hk_mem *mem1, hk_mem *mem2);
bool hk_mem_equal_str(hk_mem *mem, const char *str);
void hk_mem_log_as_string(const char *title, hk_mem *mem);
void hk_mem_log(const char *title, hk_mem *mem);

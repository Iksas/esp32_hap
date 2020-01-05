#include "hk_mem.h"
#include "hk_logging.h"

#include <string.h>

hk_mem *hk_mem_create()
{
    hk_mem *mem = malloc(sizeof(hk_mem));
    mem->size = 0;
    mem->ptr = NULL;
    return mem;
}

void hk_mem_append(hk_mem *mem, hk_mem *mem_to_append)
{
    hk_mem_append_buffer(mem, mem_to_append->ptr, mem_to_append->size);
}

void hk_mem_append_buffer(hk_mem *mem, char *data, size_t size)
{
    size_t new_size = mem->size + size;
    mem->ptr = realloc(mem->ptr, new_size);
    memcpy(mem->ptr + mem->size, data, size);
    mem->size = new_size;
}

void hk_mem_append_string(hk_mem *mem, const char *string)
{
    hk_mem_append_buffer(mem, (char *)string, strlen(string));
}

void hk_mem_append_string_terminator(hk_mem *mem)
{
    hk_mem_set(mem, mem->size + 1);
    mem->ptr[mem->size - 1] = 0;
}

void hk_mem_set(hk_mem *mem, size_t size)
{
    if (mem->size != size)
    {
        mem->ptr = realloc(mem->ptr, size);
        mem->size = size;
    }
}

void hk_mem_free(hk_mem *mem)
{
    free(mem->ptr);
    mem->ptr = NULL;
    free(mem);
}

char *hk_mem_get_str(hk_mem *mem)
{
    return strndup(mem->ptr, mem->size);
}

bool hk_mem_cmp_str(hk_mem *mem, const char *str)
{
    return strncmp(mem->ptr, str, mem->size) == 0;
}

bool hk_mem_cmp(hk_mem *mem1, hk_mem *mem2)
{
    return strncmp(mem1->ptr, mem2->ptr, mem1->size) == 0;
}

void hk_mem_log(const char *title, hk_mem *mem)
{
    hk_log_print_as_string(title, mem->ptr, mem->size);
}
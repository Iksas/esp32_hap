#include <stdlib.h>
#include "hk_ll.h"

#define hk_ll_next_ptr(ptr) (*ptr)
#define hk_ll_data_to_next_ptr(data_ptr) (((void **)data_ptr) - 1)
#define hk_ll_next_ptr_to_data(ptr) (((void **)ptr) + 1)

void *_hk_ll_init(void *next, size_t size)
{
    void **ptr = (void **)malloc(sizeof(void *) + size);
    *ptr = next;
    void *data_ptr = hk_ll_next_ptr_to_data(ptr);
    return data_ptr;
}

void *_hk_ll_next(void *data)
{
    void **next_ptr = hk_ll_data_to_next_ptr(data);
    void *next_data = *next_ptr;
    return next_data;
}

void *hk_ll_reverse(void *data)
{
    void *current_data = data, *prev_data = NULL, *next_data;
    void **current_ptr = NULL;

    while (current_data != NULL)
    {
        current_ptr = hk_ll_data_to_next_ptr(current_data);
        next_data = *current_ptr;

        *current_ptr = prev_data;
        prev_data = current_data;
        current_data = next_data;
    }

    return prev_data;
}

void *hk_ll_remove(void *list, void *data_to_remove)
{
    void *current_data = list, *prev_data = NULL;
    void **current_ptr = NULL, **prev_ptr = NULL;

    while (current_data != NULL)
    {
        if (current_data == data_to_remove)
        {
            current_ptr = hk_ll_data_to_next_ptr(current_data);
            void *next_data = _hk_ll_next(current_data);
            if (prev_data != NULL)
            {
                prev_ptr = hk_ll_data_to_next_ptr(prev_data);
                *prev_ptr = next_data;
            }
            else
            {
                list = next_data;
            }

            free(current_ptr);
            return list;
        }
        else
        {
            prev_data = current_data;
            current_data = _hk_ll_next(current_data);
        }
    }

    return list;
}

int hk_ll_count(void *data)
{
    int count = 0;
    hk_ll_foreach(data, item)
    {
        count++;
    }

    return count;
}

void hk_ll_free(void *data)
{
    void **ptr_to_free = NULL;

    while (data)
    {
        ptr_to_free = hk_ll_data_to_next_ptr(data);
        data = _hk_ll_next(data);
        free(ptr_to_free);
    }
}
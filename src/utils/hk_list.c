#include "hk_list.h"

#include "hk_logging.h"

typedef struct hk_list
{
    struct hk_list *next;
    void *value[];
} hk_list_t;

void _hk_list_append(void *list, size_t size)
{
    hk_list_t *item = malloc(sizeof(hk_list_t) + size);

    if (!item)
    {
        HK_LOGE("List: Error allocating memory for list.");
        return;
    }

    ((hk_list_t*)list)->next = item;
}

void hk_list_free(void *list)
{
    hk_list_t *tmp_list, *next;
    for (tmp_list = list; tmp_list; tmp_list = next)
    {
        tmp_list--;
        next = tmp_list->next;
        free(tmp_list);
    }
}

void *_hk_list_next(void *list)
{
    return list ? ((hk_list_t *)list)[-1].next : NULL;
}
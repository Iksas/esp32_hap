#pragma once

#include <stdlib.h>

#define hk_list_append(list) (typeof (list)) _hk_list_append(list, sizeof(*list))
#define hk_list_foreach(list, item) for (typeof (list) item = list; item; item = _hk_list_next(item))
void hk_list_free(void *list);

void _hk_list_append(void *next, size_t size);
void *_hk_list_next(void *list);

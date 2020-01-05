#pragma once

#include <stddef.h>

#define hk_ll_new(ll) (typeof (ll)) _hk_ll_new(ll, sizeof(*ll))
#define hk_ll_next(ll) (typeof (ll)) _hk_ll_next(ll)

#define hk_ll_foreach(ll, item) \
    for (typeof (ll) item = ll; item; item = _hk_ll_next(item))

void *_hk_ll_new(void *ll, size_t size);
void *_hk_ll_next(void *ll);
void hk_ll_free(void *ll);
void * hk_ll_reverse(void *ll);
void *hk_ll_remove(void *ll, void *ll_to_remove);
int hk_ll_count(void *ll);

#pragma once

#include <cJSON.h>

#include "../../include/hk_mem.h"
#include "hk_accessories_store.h"

void hk_accessories_serializer_value(hk_chr_t *chr, cJSON *j_chr);

void hk_accessories_serializer_accessories(hk_mem *out);
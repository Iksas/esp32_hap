#pragma once

#include <cJSON.h>

#include "../../include/hk_mem.h"
#include "hk_accessories_store.h"

esp_err_t hk_accessories_serializer_value(hk_chr_t *chr, cJSON *j_chr);

esp_err_t hk_accessories_serializer_accessories(hk_mem *out);
#pragma once

#include "../../common/hk_accessories_store.h"
#include "../../utils/hk_mem.h"

#include <cJSON.h>

void hk_accessories_serializer_value(hk_characteristic_t *characteristic, cJSON *j_characteristic);

void hk_accessories_serializer_accessories(hk_mem *out);
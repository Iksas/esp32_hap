#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"
esp_err_t hk_accessory_id_get(hk_mem *id);
esp_err_t hk_accessory_id_get_serialized(hk_mem *id);
esp_err_t hk_accessory_id_reset();

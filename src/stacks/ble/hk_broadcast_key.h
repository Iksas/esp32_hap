#pragma once

#include <esp_err.h>

#include "../../include/hk_mem.h"

esp_err_t hk_broadcast_key_get(hk_mem *accessory_shared_secret, hk_mem *broadcast_key);
esp_err_t hk_broadcast_key_reset(hk_mem *accessory_shared_secret, hk_mem *broadcast_key);
#pragma once

#include "hk_mem.h"

#include <stdbool.h>
#include <esp_system.h>

esp_err_t hk_util_get_accessory_id(uint8_t id[]);
esp_err_t hk_util_get_accessory_id_serialized(hk_mem* id);
bool hk_util_string_ends_with(const char *str, const char *suffix);
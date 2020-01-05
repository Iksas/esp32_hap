#pragma once

#include "hk_mem.h"

#include <stdbool.h>

size_t hk_util_get_accessory_id(hk_mem* id);
bool hk_util_string_ends_with(const char *str, const char *suffix);
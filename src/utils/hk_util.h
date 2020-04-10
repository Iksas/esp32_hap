#pragma once

#include <stdbool.h>
#include <esp_system.h>

#include "hk_mem.h"
#include "hk_errors.h"

#define RUN_AND_CHECK(ret, func, args...) \
if(!ret) \
{ \
    ret = func(args); \
    if (ret)\
    {\
        HK_LOGE("Error executing: %s (%d)", hk_error_to_name(ret), ret); \
    }  \
}

esp_err_t hk_util_get_accessory_id(uint8_t id[]);
esp_err_t hk_util_get_accessory_id_serialized(hk_mem* id);
bool hk_util_string_ends_with(const char *str, const char *suffix);
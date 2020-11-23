#pragma once

#include <stdbool.h>
#include <esp_system.h>

#include "hk_logging.h"
#include "hk_mem.h"

/**
 * @file hk_tlv.h
 *
 * Functions to work with type length value types.
 */

/**
 * @brief Runs a function with parameters and checks for errors.
 */
#define RUN_AND_CHECK(ret, func, args...) \
if(!ret) \
{ \
    ret = func(args); \
    if (ret)\
    {\
        HK_LOGE("Error executing: %s (%d)", esp_err_to_name(ret), ret); \
    }  \
}

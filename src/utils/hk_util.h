#pragma once

#include <stdbool.h>
#include <esp_system.h>

#include "hk_logging.h"
#include "hk_mem.h"

#define RUN_AND_CHECK(ret, func, args...) \
if(!ret) \
{ \
    ret = func(args); \
    if (ret)\
    {\
        HK_LOGE("Error executing: %s (%d)", hk_error_to_name(ret), ret); \
    }  \
}

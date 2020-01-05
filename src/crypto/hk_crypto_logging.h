#pragma once

#include "../utils/hk_logging.h"
#define WOLFSSL_USER_SETTINGS
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/error-crypt.h>

#define HK_CRYPOT_ERR(message, error_code) HK_LOGE("Crypto: %s. %s (%d).", message, wc_GetErrorString(error_code), error_code);
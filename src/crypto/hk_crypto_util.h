/**
 * @file hk_crypto_util.h
 *
 * Utils used in all crypto files.
 */

#pragma once

#define WOLFSSL_USER_SETTINGS
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/error-crypt.h>

#include "../utils/hk_logging.h"

/**
 * @brief Runs the given function and checks the return.
 */
#define HK_CRYPTO_RUN_AND_CHECK(ret, func, args...) \
if(!ret) \
{ \
    ret = func(args); \
    if (ret)\
    {\
        HK_LOGE("Error executing: %s (%d)", wc_GetErrorString(ret), ret); \
    }  \
}

#define HK_CRYPOT_ERR(message, error_code) HK_LOGE("%s. %s (%d).", message, wc_GetErrorString(error_code), error_code);
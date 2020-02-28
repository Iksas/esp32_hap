#pragma once

#include "../include/hk_mem.h"

enum hk_hkdf_types {
    HK_HKDF_PAIR_SETUP_ENCRYPT,
    HK_HKDF_PAIR_SETUP_CONTROLLER,
    HK_HKDF_PAIR_SETUP_ACCESSORY,
    HK_HKDF_PAIR_VERIFY_ENCRYPT,
    HK_HKDF_CONTROL_READ,
    HK_HKDF_CONTROL_WRITE,
    HK_HKDF_LENGTH,
};

size_t hk_hkdf(enum hk_hkdf_types type, hk_mem *key_in, hk_mem* key_out);
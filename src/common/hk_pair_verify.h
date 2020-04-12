#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"
#include "hk_pair_verify_keys.h"

int hk_pair_verify(hk_mem *request, hk_pair_verify_keys_t *keys, hk_mem *result, bool *is_session_encrypted);

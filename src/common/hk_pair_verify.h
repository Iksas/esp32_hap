#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"
#include "hk_conn_key_store.h"

int hk_pair_verify(hk_mem *request, hk_mem *result, hk_conn_key_store_t *keys, bool *is_session_encrypted);

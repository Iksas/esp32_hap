#pragma once

#include <esp_err.h>
#include <stdlib.h>

#include "../include/hk_mem.h"
#include "../utils/hk_tlv.h"
#include "hk_conn_key_store.h"

esp_err_t hk_pair_setup(hk_mem *request, hk_mem *response, hk_conn_key_store_t *keys, hk_mem *device_id, bool *is_paired);

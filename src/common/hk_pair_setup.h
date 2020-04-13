#pragma once

#include "../include/hk_mem.h"
#include "../utils/hk_tlv.h"

#include <esp_err.h>
#include <stdlib.h>

esp_err_t hk_pair_setup(hk_mem *request, hk_mem *response, hk_mem *device_id, bool *is_paired);

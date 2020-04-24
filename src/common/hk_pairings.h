#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"

esp_err_t hk_pairings(hk_mem *request, hk_mem *response, bool *kill_session, bool *is_paired);

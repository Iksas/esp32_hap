
#pragma once

#include "../../../include/hk_mem.h" 

#include "../hk_session.h"

esp_err_t hk_chr_write_response(const ble_uuid128_t *chr_uuid, hk_session_t *session);

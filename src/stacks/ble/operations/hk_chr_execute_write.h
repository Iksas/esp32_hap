
#pragma once

#include "../../../utils/hk_mem.h" 

#include "../hk_session.h"

void hk_chr_execute_write_response(const ble_uuid128_t *chr_uuid, hk_session_t *session, hk_mem* response);

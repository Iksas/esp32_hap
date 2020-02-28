
#pragma once

#include "../../../utils/hk_mem.h" 

#include "../hk_session.h"

void hk_service_signature_read_response(const ble_uuid128_t *characteristic_uuid, hk_session_t *session, hk_mem* response);

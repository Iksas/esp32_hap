#pragma once

#include "../../common/hk_pair_verify.h"
#include "hk_connection.h"

esp_err_t hk_connection_security_decrypt(hk_connection_t *connection, hk_mem *in, hk_mem *out);
esp_err_t hk_connection_security_encrypt(hk_connection_t *connection, hk_mem *in, hk_mem *out);
#pragma once

#include "../../common/hk_pair_verify.h"

hk_pair_verify_keys_t* hk_session_security_keys_get();
void hk_session_security_init_or_reset();
esp_err_t hk_session_security_decrypt(hk_mem *in, hk_mem *out);
esp_err_t hk_session_security_encrypt(hk_mem *in, hk_mem *out);
bool hk_session_security_is_secured_get();
void hk_session_security_is_secured_set(bool is_encrypted);
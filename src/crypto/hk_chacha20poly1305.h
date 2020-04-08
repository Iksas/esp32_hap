#pragma once

#include "../include/hk_mem.h"
#include "esp_err.h"

#define HK_CHACHA_RESUME_MSG1 "\x0\x0\x0\x0PR-Msg01"
#define HK_CHACHA_RESUME_MSG2 "\x0\x0\x0\x0PR-Msg02"
#define HK_CHACHA_VERIFY_MSG2 "\x0\x0\x0\x0PV-Msg02"
#define HK_CHACHA_VERIFY_MSG3 "\x0\x0\x0\x0PV-Msg03"
#define HK_CHACHA_SETUP_MSG5 "\x0\x0\x0\x0PS-Msg05"
#define HK_CHACHA_SETUP_MSG6 "\x0\x0\x0\x0PS-Msg06"

esp_err_t hk_chacha20poly1305_encrypt(hk_mem *key, const char *nonce, hk_mem *message, hk_mem *encrypted);
esp_err_t hk_chacha20poly1305_encrypt_buffer(hk_mem *key, const char *nonce, char *aad, size_t aad_size, char *message, char *encrypted, size_t message_size);
esp_err_t hk_chacha20poly1305_decrypt(hk_mem *key, const char *nonce, hk_mem *encrypted, hk_mem *message);
esp_err_t hk_chacha20poly1305_decrypt_buffer(hk_mem *key, const char *nonce, char *aad, size_t aad_size, char *encrypted,
                                          char *message, size_t message_size);
esp_err_t hk_chacha20poly1305_verify_auth_tag(hk_mem *key, const char *nonce, hk_mem *auth_tag);
esp_err_t hk_chacha20poly1305_caluclate_auth_tag_without_message(hk_mem *key, const char* nonce, hk_mem *auth_tag);
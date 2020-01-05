#pragma once

#include "../utils/hk_mem.h"
#include "../utils/hk_res.h"

typedef struct
{
    void *internal;
} hk_ed25519_key_t;

esp_err_t hk_ed25519_generate_key(hk_ed25519_key_t *key);
esp_err_t hk_ed25519_generate_key_from_keys(hk_ed25519_key_t *key, hk_mem *private_key, hk_mem *public_key);
esp_err_t hk_ed25519_generate_key_from_public_key(hk_ed25519_key_t *key, hk_mem *public_key);
esp_err_t hk_ed25519_export_public_key(hk_ed25519_key_t *key, hk_mem *public_key);
esp_err_t hk_ed25519_export_private_key(hk_ed25519_key_t *key, hk_mem *private_key);
esp_err_t hk_ed25519_verify(hk_ed25519_key_t *key, hk_mem *message, hk_mem *signature);
esp_err_t hk_ed25519_sign(hk_ed25519_key_t *key, hk_mem *message, hk_mem *signature);

hk_ed25519_key_t *hk_ed25519_init_key();
void hk_ed25519_free_key(hk_ed25519_key_t *key);
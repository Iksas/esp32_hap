#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"

typedef struct
{
    void *internal;
} hk_ed25519_key_t;

esp_err_t hk_ed25519_update_from_random(hk_ed25519_key_t *key);
esp_err_t hk_ed25519_update_from_random_keys(hk_ed25519_key_t *key, hk_mem *private_key, hk_mem *public_key);
esp_err_t hk_ed25519_update_from_random_from_public_key(hk_ed25519_key_t *key, hk_mem *public_key);
esp_err_t hk_ed25519_export_public_key(hk_ed25519_key_t *key, hk_mem *public_key);
esp_err_t hk_ed25519_export_private_key(hk_ed25519_key_t *key, hk_mem *private_key);
esp_err_t hk_ed25519_verify(hk_ed25519_key_t *key, hk_mem *signature, hk_mem *message);
esp_err_t hk_ed25519_sign(hk_ed25519_key_t *key, hk_mem *message, hk_mem *signature);

hk_ed25519_key_t *hk_ed25519_init();
void hk_ed25519_free(hk_ed25519_key_t *key);
#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"

typedef struct
{
    void *internal;
} hk_curve25519_key_t;

hk_curve25519_key_t *hk_curve25519_init();
esp_err_t hk_curve25519_update_from_random(hk_curve25519_key_t *key);
esp_err_t hk_curve25519_update_from_public_key(hk_mem *public_key, hk_curve25519_key_t *key);
esp_err_t hk_curve25519_export_public_key(hk_curve25519_key_t *key, hk_mem *public_key);
esp_err_t hk_curve25519_calculate_shared_secret(hk_curve25519_key_t *key1, hk_curve25519_key_t *key2, hk_mem *shared_secret);
void hk_curve25519_free(hk_curve25519_key_t *key);
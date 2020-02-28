#pragma once

#include "../include/hk_mem.h"

typedef struct
{
    void *internal;
} hk_curve25519_key_t;

hk_curve25519_key_t *hk_curve25519_init_key();
size_t hk_curve25519_generate_key(hk_curve25519_key_t *key);
size_t hk_curve25519_generate_key_from_public_key(hk_curve25519_key_t *key, hk_mem *public_key);
size_t hk_curve25519_export_public_key(hk_curve25519_key_t *key, hk_mem *public_key);
size_t hk_curve25519_shared_secret(hk_curve25519_key_t *key1, hk_curve25519_key_t *key2, hk_mem *shared_secret);
void hk_curve25519_free_key(hk_curve25519_key_t *key);
#pragma once

#include "../include/hk_mem.h"

typedef struct
{
    void *internal;
} hk_srp_key_t;

size_t hk_srp_generate_key(hk_srp_key_t *key, const char *username, const char *password);
size_t hk_srp_export_public_key(hk_srp_key_t *key, hk_mem *public_key);
size_t hk_srp_export_salt(hk_srp_key_t *key, hk_mem *salt);
size_t hk_srp_verify(hk_srp_key_t *key, hk_mem *proof);
size_t hk_srp_compute_key(hk_srp_key_t *key, hk_mem *server_public_key, hk_mem *client_public_key);
size_t hk_srp_export_proof(hk_srp_key_t *key, hk_mem *proof);
size_t hk_srp_export_private_key(hk_srp_key_t *key, hk_mem *private_key);

hk_srp_key_t *hk_srp_init_key();
void hk_srp_free_key(hk_srp_key_t *key);
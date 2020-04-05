#pragma once

#include "../include/hk_mem.h"

typedef struct hk_pair_verify_keys
{
    // these pointers are created by session (ip) or transaction (ble)
    hk_mem *response_key;
    hk_mem *request_key;
    hk_mem *shared_secret;

    //the following variables are used during verification process, they are allocated and freed during it
    hk_mem *session_key;
    hk_mem *accessory_curve_public_key;
    hk_mem *device_curve_public_key;
} hk_pair_verify_keys_t;

hk_pair_verify_keys_t *hk_pair_verify_keys_init();
void hk_pair_verify_keys_free(hk_pair_verify_keys_t *keys);
void hk_pair_verify_keys_reset(hk_pair_verify_keys_t *keys);
#pragma once

#include "../include/hk_mem.h"
#include "../crypto/hk_srp.h"

typedef struct
{
    // fields are created per connection
    hk_mem *response_key;
    hk_mem *request_key;
    hk_mem *accessory_shared_secret;

    // fields used during verification process
    hk_mem *session_key;
    hk_mem *accessory_session_key_public;
    hk_mem *device_session_key_public;

    // fields used during setup process, they are allocated and freed by hk_pair_setup
    hk_mem *pair_setup_public_key;
    hk_srp_key_t *pair_setup_srp_key;
} hk_conn_key_store_t;

hk_conn_key_store_t *hk_conn_key_store_init();
void hk_conn_key_store_free(hk_conn_key_store_t *keys);
void hk_conn_key_store_reset(hk_conn_key_store_t *keys);

void hk_conn_key_store_verify_reset(hk_conn_key_store_t *keys);
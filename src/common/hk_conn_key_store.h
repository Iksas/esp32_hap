/**
 * @file hk_conn_key_store.h
 *
 * Holds the connection keys.
 */

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


/**
 * @brief Initializes the keys of a connection
 *
 * Initializes the keys of a connection.
 */
hk_conn_key_store_t *hk_conn_key_store_init();

/**
 * @brief Frees the keys of a connection
 *
 * Frees the keys of a connection.
 *
 * @param keys The keys.
 */
void hk_conn_key_store_free(hk_conn_key_store_t *keys);

/**
 * @brief Resets the keys of a connection.
 *
 * Resets all keys of a connection. That should be done on new pair verify.
 *
 * @param keys The keys.
 */
void hk_conn_key_store_reset(hk_conn_key_store_t *keys);
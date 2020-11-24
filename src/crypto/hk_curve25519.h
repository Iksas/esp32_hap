/**
 * @file hk_curve25519.h
 *
 * Functions for working with curve 25519 crypto algorithm.
 */

#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"

/**
 * @brief Typedef for a curve25519 key.
 */
typedef struct
{
    void *internal;
} hk_curve25519_key_t;

/**
 * @brief Initializes a key.
 *
 * Initializes a key.
 * 
 * @return Returns the key.
 */
hk_curve25519_key_t *hk_curve25519_init();

/**
 * @brief Creates a new random key.
 *
 * Creates a new random key.
 *
 * @param key The encryption key.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_curve25519_update_from_random(hk_curve25519_key_t *key);

/**
 * @brief Creates a new key from a public key.
 *
 * Creates a new key from a public key.
 *
 * @param public_key The public key.
 * @param key The encryption key.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_curve25519_update_from_public_key(hk_mem *public_key, hk_curve25519_key_t *key);

/**
 * @brief Exports the public key.
 *
 * Exports the public key.
 *
 * @param key The encryption key.
 * @param public_key The public key.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_curve25519_export_public_key(hk_curve25519_key_t *key, hk_mem *public_key);

/**
 * @brief Calculates the shared secret.
 *
 * Calculates the shared secret.
 *
 * @param key1 The first key.
 * @param key2 The second key.
 * @param shared_secret The shared secret.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_curve25519_calculate_shared_secret(hk_curve25519_key_t *key1, hk_curve25519_key_t *key2, hk_mem *shared_secret);

/**
 * @brief Frees the ressources used by the key.
 *
 * Frees the ressources used by the key.
 *
 * @param key The encryption key.
 */
void hk_curve25519_free(hk_curve25519_key_t *key);
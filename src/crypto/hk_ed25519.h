/**
 * @file hk_ed25519.h
 *
 * Functions for working with ed25519 crypto algorithm.
 */

#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"

/**
 * @brief Typedef for a ed25519 key.
 */
typedef struct
{
    void *internal;
} hk_ed25519_key_t;

/**
 * @brief Initializes the key.
 *
 * Initializes the key.
 * 
 * @return Returns the key.
 */
hk_ed25519_key_t *hk_ed25519_init();

/**
 * @brief Initializes a random key.
 *
 * Initializes a random key.
 * 
 * @param key The encryption key.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_ed25519_init_from_random(hk_ed25519_key_t *key);

/**
 * @brief Initializes a key from private and public key.
 *
 * Initializes a key from private and public key.
 * 
 * @param key The encryption key.
 * @param private_key The private key.
 * @param public_key The public key.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_ed25519_init_from_keys(hk_ed25519_key_t *key, hk_mem *private_key, hk_mem *public_key);

/**
 * @brief Initializes a key from public key.
 *
 * Initializes a key from public key.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_ed25519_init_from_public_key(hk_ed25519_key_t *key, hk_mem *public_key);

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
esp_err_t hk_ed25519_export_public_key(hk_ed25519_key_t *key, hk_mem *public_key);

/**
 * @brief Exports the private key.
 *
 * Exports the private key.
 * 
 * @param key The encryption key.
 * @param private_key The private key.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_ed25519_export_private_key(hk_ed25519_key_t *key, hk_mem *private_key);

/**
 * @brief Verifies the message.
 *
 * Verifies the message.
 * 
 * @param key The encryption key.
 * @param signature The signature.
 * @param message The message.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_ed25519_verify(hk_ed25519_key_t *key, hk_mem *signature, hk_mem *message);

/**
 * @brief Creates a signature for the message.
 *
 * Creates a signature for the message.
 * 
 * @param key The encryption key.
 * @param message The message.
 * @param signature The created signature.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_ed25519_sign(hk_ed25519_key_t *key, hk_mem *message, hk_mem *signature);

/**
 * @brief Frees all ressources of the key.
 *
 * Frees all ressources of the key.
 * 
 * @param key The key.
 */
void hk_ed25519_free(hk_ed25519_key_t *key);
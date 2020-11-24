/**
 * @file hk_srp.h
 *
 * Functions for working with srp crypto algorithm.
 */

#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"

/**
 * @brief Typedef for a srp key.
 */
typedef struct
{
    void *internal;
} hk_srp_key_t;

/**
 * @brief Initializes a key.
 *
 * Initializes a key.
 * 
 * @return Returns the key.
 */
hk_srp_key_t *hk_srp_init_key();

/**
 * @brief Generates a key.
 *
 * Generates a key.
 * 
 * @param key The encryption key.
 * @param username The user name.
 * @param password The password.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_srp_generate_key(hk_srp_key_t *key, const char *username, const char *password);

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
esp_err_t hk_srp_export_public_key(hk_srp_key_t *key, hk_mem *public_key);

/**
 * @brief Exports the salt.
 *
 * Exports the salt.
 * 
 * @param key The encryption key.
 * @param salt The salt.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_srp_export_salt(hk_srp_key_t *key, hk_mem *salt);

/**
 * @brief Verifies a key.
 *
 * Verifies a key.
 * 
 * @param key The encryption key.
 * @param proof The proof.
 * @param valid True if the key is valid.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_srp_verify(hk_srp_key_t *key, hk_mem *proof, bool *valid);

/**
 * @brief Computes a key.
 *
 * Computes a key.
 * 
 * @param key The encryption key.
 * @param server_public_key The server public key.
 * @param client_public_key The client public key.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_srp_compute_key(hk_srp_key_t *key, hk_mem *server_public_key, hk_mem *client_public_key);

/**
 * @brief Exports the proof.
 *
 * Exports the proof.
 * 
 * @param key The encryption key.
 * @param proof The exported proof.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_srp_export_proof(hk_srp_key_t *key, hk_mem *proof);

/**
 * @brief Exports a private key.
 *
 * Exports a private key.
 * 
 * @param key The encryption key.
 * @param private_key The exported private key.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_srp_export_private_key(hk_srp_key_t *key, hk_mem *private_key);

/**
 * @brief Frees all ressources used by the key.
 *
 * Frees all ressources used by the key.
 * 
 * @param key The encryption key.
 */
void hk_srp_free_key(hk_srp_key_t *key);
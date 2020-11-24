/**
 * @file hk_pairings_store.h
 *
 * Stores the pairing information.
 */

#pragma once

#include "../include/hk_mem.h"

#include <esp_err.h>
#include <stdbool.h>

/**
 * @brief Adds a pairing
 *
 * Adds a pairing.
 *
 * @param device_id The device id this pairing is for.
 * @param device_ltpk The device long term public key.
 * @param is_admin A flag saying if this is an admin pairing.
 * 
 * @return Returns ESP_OK on success.
 */
esp_err_t hk_pairings_store_add(hk_mem* device_id, hk_mem* device_ltpk, bool is_admin);

/**
 * @brief Removes a pairing
 *
 * Removes all pairingings of a given device.
 *
 * @param device_id The device id to delete pairings for.
 * 
 * @return Returns ESP_OK on success.
 */
esp_err_t hk_pairings_store_remove(hk_mem *device_id);

/**
 * @brief Check if a device pairing exists
 *
 * Check if a device pairing exists
 *
 * @param device_id The device id this pairing is for.
 * @param exists True if pairing exists.
 * 
 * @return Returns ESP_OK on success.
 */
esp_err_t hk_pairings_store_device_exists(hk_mem *device_id, bool *exists);

/**
 * @brief Check if a device pairing is an admin pairing
 *
 * Check if a device pairing is an admin pairing.
 *
 * @param device_id The device id this pairing is for.
 * @param is_admin True if admin pairing.
 * 
 * @return Returns ESP_OK on success.
 */
esp_err_t hk_pairings_store_is_admin(hk_mem *device_id, bool *is_admin);

/**
 * @brief Checks if store holds an admin pairing
 *
 * Checks if store holds an admin pairing.
 *
 * @param is_admin True if admin pairing exists.
 * 
 * @return Returns ESP_OK on success.
 */
esp_err_t hk_pairings_store_has_admin_pairing(bool *is_admin);

/**
 * @brief Checks if store has any pairing
 *
 * Checks if store has any pairing.
 *
 * @param has_pairing True if at least one pairing exists.
 * 
 * @return Returns ESP_OK on success.
 */
esp_err_t hk_pairings_store_has_pairing(bool *has_pairing);

/**
 * @brief Removes all pairings
 *
 * Removes all pairings.
 * 
 * @return Returns ESP_OK on success.
 */
esp_err_t hk_pairings_store_remove_all();

/**
 * @brief Returns the long term public key of a device
 *
 * Returns the long term public key of a device.
 *
 * @param device_id The device id this pairing is for.
 * @param device_ltpk The device long term public key.
 * 
 * @return Returns ESP_OK on success.
 */
esp_err_t hk_pairings_store_ltpk_get(hk_mem *device_id, hk_mem *device_ltpk);


/**
 * @brief Logs all pairings
 *
 * Logs all pairings with severity INFO.
 * 
 * @return Returns ESP_OK on success.
 */
esp_err_t hk_pairings_log_devices();

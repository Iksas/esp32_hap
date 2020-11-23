#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <esp_err.h>
#include <nvs_flash.h>

#include "hk_mem.h"

/**
 * @file hk_store.h
 *
 * A fascade to easily access the non volantile store of esp32.
 */

/**
 * @brief Initalize the store.
 *
 * Initalize the store. Has to be called before all other methods work.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_store_init();

/**
 * @brief Returns a stored uint8_t.
 *
 * Returns a stored uint8_t.
 *
 * @param key The key to access the value.
 * @param value A pointer to hold the value.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_store_u8_get(const char *key, uint8_t *value);

/**
 * @brief Returns a stored uint8_t.
 *
 * Returns a stored uint8_t.
 *
 * @param key The key to access the value.
 * @param value A pointer to hold the value.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_store_u8_set(const char *key, uint8_t value);

/**
 * @brief Sets a uint8_t.
 *
 * Sets a stored uint8_t.
 *
 * @param key The key to access the value.
 * @param value The value.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_store_u16_get(const char *key, uint16_t *value);

/**
 * @brief Initalize a list.
 *
 * Allocates the first item in the list.
 *
 * @param ll A pointer to the first item of the list.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_store_u16_set(const char *key, uint16_t value);

/**
 * @brief Returns a stored hk_mem.
 *
 * Returns a stored hk_mem.
 *
 * @param key The key to access the value.
 * @param value A pointer to hold the value.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_store_blob_get(const char *key, hk_mem *value);

/**
 * @brief Sets a hk_mem.
 *
 * Sets a hk_mem.
 *
 * @param key The key to access the value.
 * @param value A pointer to hold the value.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_store_blob_set(const char *key, hk_mem *value);

/**
 * @brief Removes the value of the given key from the store.
 *
 * Removes the value of the given key from the store.
 *
 * @param key The key to access the value.
 * 
 * @return Returns an esp_err_t result.
 */
esp_err_t hk_store_erase(const char *key);

/**
 * @brief Frees all ressources allocated by the store.
 *
 * Frees all ressources allocated by the store.
 */
void hk_store_free();

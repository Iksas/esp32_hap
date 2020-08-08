#pragma once

#include "hk_srvs.h"
#include "hk_chrs.h"
#include "hk_categories.h"
#include "hk_mem.h"
#include <stdlib.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @file hk.h
 *
 * Functions for initializing this library.
 */

/**
 * @brief Initalize homekit
 *
 * Initializes homekit.
 *
 * @param name The name of the device.
 * @param category The well known category of the device.
 * @param code The code to connect to the device.
 */
void hk_init(const char *name, const hk_categories_t category, const char *code);

/**
 * @brief Starts the setup  proccess
 *
 * Starts the setup  proccess.
 */
void hk_setup_start();

/**
 * @brief Setup an accessory
 *
 * Setup up an accessory. Be sure to call hk_setup_start before.
 *
 * @param name The name of the accessory.
 * @param manufacturer The manufacturer of the accessory.
 * @param model The model name of the accessory.
 * @param serial_number The serial number of the accessory.
 * @param revision The revision of the accessory.
 * @param identify A function that is called, if the user requests identification.
 */
void hk_setup_add_accessory(const char *name, const char *manufacturer, const char *model, const char *serial_number, const char *revision, void (*identify)());

/**
 * @brief Add a service
 *
 * Adds a service. It will hold the characteristics.
 *
 * @param srv_type The type of the service.
 * @param primary If this is the primary service.
 * @param hidden If this is a hidden service.
 */
void hk_setup_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden);

/**
 * @brief Add a characteristic
 *
 * Adds a characteristic. It defines the features of your device.
 *
 * @param chr_type The type of the characteristic.
 * @param read The function called if the characteristic is read. NULL if characteristec cannot be read.
 * @param write The function called if the characteristic is written. NULL if characteristec cannot be written.
 * @param can_notify True if the property can notify homekit for changes.
 */
void *hk_setup_add_chr(hk_chr_types_t chr_type, esp_err_t (*read)(hk_mem* response), esp_err_t (*write)(hk_mem* request), bool can_notify);

/**
 * @brief Finish setup
 *
 * Has to be called, after all services and characteristics have been added.
 */
void hk_setup_finish();

/**
 * @brief Reset homekit
 *
 * Resets all homekit data. Espesially the pairing information. After calling reset, the device should be restarted.
 */
void hk_reset();

/**
 * @brief Notify homekit that a property has changed
 *
 * Calling this method triggers homekit to read a new value and notify all listening devices.
 * 
 * @param chr The characteristic handle, returned by hk_setup_add_chr;
 */
void hk_notify(void *chr);
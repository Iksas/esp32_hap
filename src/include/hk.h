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
void hk_setup_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden);
void *hk_setup_add_chr(hk_chr_types_t type, esp_err_t (*read)(hk_mem* response), esp_err_t (*write)(hk_mem* request), bool can_notify);
void hk_setup_finish();
void hk_reset();

void hk_notify(void *chr);
#pragma once

#include "hk_mem.h"
#include "esp_err.h"

/**
 * @brief Set up a switch device
 *
 * Does everything needed to setup a homekit switch.
 *
 * @param name The name of the device.
 * @param manufacturer The manufacturer of the device.
 * @param model The model of the device.
 * @param serial_number The serial number of the device.
 * @param revision The revision of device.
 * @param identify The method to be called if the user wants to identify the device.
 * @param read The method that is called by homekit to aquire the current status of the switch.
 * @param write The method to be calle if the user has changed the status of the switch. On or off.
 */
void *hk_setup_add_switch(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(),
    esp_err_t (*read)(hk_mem* response),
    esp_err_t (*write)(hk_mem* request));

/**
 * @brief Set up a motion sensor device
 *
 * Does everything needed to setup a homekit motion sensor.
 *
 * @param name The name of the device.
 * @param manufacturer The manufacturer of the device.
 * @param model The model of the device.
 * @param serial_number The serial number of the device.
 * @param revision The revision of device.
 * @param identify The method to be called if the user wants to identify the device.
 * @param read The method that is called by homekit to aquire the current status of the motion sensor.
 */
void *hk_setup_add_motion_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(), 
    esp_err_t (*read)(hk_mem* response));
/**
 * @file hk_fascade.h
 *
 * A fascade to make it easier to create simple homekit devices.
 */

#pragma once

#include <esp_err.h>

#include "hk_mem.h"

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
 * @param chr_ptr A pointer to the characteristic.
 */
esp_err_t hk_setup_add_switch(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(),
    esp_err_t (*read)(hk_mem* response),
    esp_err_t (*write)(hk_mem* request),
    void **chr_ptr);

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
 * @param chr_ptr A pointer to the characteristic.
 */
esp_err_t hk_setup_add_motion_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(),
    esp_err_t (*read)(hk_mem* response),
    void **chr_ptr);


/**
 * @brief Set up a temperature sensor device
 *
 * Does everything needed to setup a homekit temperature sensor.
 *
 * @param name The name of the device.
 * @param manufacturer The manufacturer of the device.
 * @param model The model of the device.
 * @param serial_number The serial number of the device.
 * @param revision The revision of device.
 * @param identify The method to be called if the user wants to identify the device.
 * @param read_temp The method that is called by homekit to aquire the current temperature.
 * @param chr_ptr A pointer to the characteristic.
 */
esp_err_t hk_setup_add_temperature_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(),
    esp_err_t (*read_temp)(hk_mem* response),
    void **chr_ptr);


/**
 * @brief Set up a simple air quality sensor device
 *
 * Does everything needed to setup a homekit air quality sensor.
 * This simple sensor only transmits the air quality in
 * five different levels, and no detailed data.
 *
 * @param name The name of the device.
 * @param manufacturer The manufacturer of the device.
 * @param model The model of the device.
 * @param serial_number The serial number of the device.
 * @param revision The revision of device.
 * @param identify The method to be called if the user wants to identify the device.
 * @param read_air_quality The method that is called by homekit to aquire the current air quality.
 * @param chr_ptr A pointer to the characteristic.
 */
esp_err_t hk_setup_add_simple_air_quality_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(),
    esp_err_t (*read_air_quality)(hk_mem* response),
    void **chr_ptr);


/**
 * @brief Set up an air quality sensor supporting all possible characteristics
 *
 * Does everything needed to setup a homekit air quality sensor.
 * This sensor supports all possible air quality characteristics:
 *
 * - air quality
 * - ozone density
 * - nitrogen dioxide density
 * - sulphur dioxode density
 * - PM2.5 density
 * - PM10 density
 * - VOC density
 *
 * as well as the following status characteristics:
 *
 * - active status
 * - fault status
 * - tampered status
 * - battery status
 *
 * @param name The name of the device.
 * @param manufacturer The manufacturer of the device.
 * @param model The model of the device.
 * @param serial_number The serial number of the device.
 * @param revision The revision of device.
 * @param identify The method to be called if the user wants to identify the device.
 * @param read_air_quality
 * @param read_ozone_density
 * @param read_nitrogen_dioxode_density
 * @param read_sulphir_dioxide_density
 * @param read_pm25_density
 * @param read_pm10_density
 * @param read_voc_density
 * @param read_active_status
 * @param read_tampered_status
 * @param read_battery_status
 * @param chr_air_quality_ptr
 * @param chr_ozone_density_ptr
 * @param chr_nitrogen_dioxide_ptr
 * @param chr_sulphur_dioxode_ptr
 * @param chr_pm25_ptr
 * @param chr_pm10_ptr
 * @param chr_voc_ptr
 * @param chr_active_ptr
 * @param chr_tampered_ptr
 * @param chr_battery_ptr
 */
esp_err_t hk_setup_add_full_air_quality_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(),
    esp_err_t (*read_air_quality)(hk_mem* response),
    esp_err_t (*read_ozone_density)(hk_mem* response),
    esp_err_t (*read_nitrogen_dioxode_density)(hk_mem* response),
    esp_err_t (*read_sulphir_dioxide_density)(hk_mem* response),
    esp_err_t (*read_pm25_density)(hk_mem* response),
    esp_err_t (*read_pm10_density)(hk_mem* response),
    esp_err_t (*read_voc_density)(hk_mem* response),
    esp_err_t (*read_active_status)(hk_mem* response),
    esp_err_t (*read_tampered_status)(hk_mem* response),
    esp_err_t (*read_battery_status)(hk_mem* response),
    void **chr_air_quality_ptr,
    void **chr_ozone_density_ptr,
    void **chr_nitrogen_dioxide_ptr,
    void **chr_sulphur_dioxode_ptr,
    void **chr_pm25_ptr,
    void **chr_pm10_ptr,
    void **chr_voc_ptr,
    void **chr_active_ptr,
    void **chr_tampered_ptr,
    void **chr_battery_ptr
);


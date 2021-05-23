#include "../include/hk.h"
#include "../include/hk_fascade.h"

#include <stdbool.h>

esp_err_t hk_setup_add_switch(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(),
    esp_err_t (*read)(hk_mem* response),
    esp_err_t (*write)(hk_mem* request),
    void **chr_ptr)
{
    hk_setup_start();
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, identify);
    hk_setup_add_srv(HK_SRV_SWITCH, true, false);
    hk_setup_add_chr(HK_CHR_ON, read, write, true, chr_ptr);
    hk_setup_finish();

    return ESP_OK;
}

esp_err_t hk_setup_add_motion_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(),
    esp_err_t (*read)(hk_mem* response),
    void** chr_ptr)
{
    hk_setup_start();
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, identify);
    hk_setup_add_srv(HK_SRV_MOTION_SENSOR, true, false);
    hk_setup_add_chr(HK_CHR_MOTION_DETECTED, read, NULL, true, chr_ptr);
    hk_setup_finish();

    return ESP_OK;
}


esp_err_t hk_setup_add_temperature_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(),
    esp_err_t (*read_temp)(hk_mem* response),
    void **chr_ptr)
{
    hk_setup_start();
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, identify);
    hk_setup_add_srv(HK_SRV_TEMPERATURE_SENSOR, true, false);
    hk_setup_add_chr(HK_CHR_CURRENT_TEMPERATURE, read_temp, NULL, true, chr_ptr);
    hk_setup_finish();

    return ESP_OK;
}

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


esp_err_t hk_setup_add_simple_air_quality_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    void (*identify)(),
    esp_err_t (*read_air_quality)(hk_mem* response),
    void **chr_ptr)
{
    hk_setup_start();
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, identify);
    hk_setup_add_srv(HK_SRV_AIR_QUALITY_SENSOR, true, false);
    hk_setup_add_chr(HK_CHR_AIR_QUALITY, read_air_quality, NULL, true, chr_ptr);
    hk_setup_finish();

    return ESP_OK;
}



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
    void **chr_battery_ptr)
{
    hk_setup_start();
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, identify);
    hk_setup_add_srv(HK_SRV_AIR_QUALITY_SENSOR, true, false);

    hk_setup_add_chr(HK_CHR_AIR_QUALITY, read_air_quality, NULL, true, chr_air_quality_ptr);
    hk_setup_add_chr(HK_CHR_OZONE_DENSITY, read_ozone_density, NULL, true, chr_ozone_density_ptr);
    hk_setup_add_chr(HK_CHR_NITROGEN_DIOXIDE_DENSITY, read_nitrogen_dioxode_density, NULL, true, chr_nitrogen_dioxide_ptr);
    hk_setup_add_chr(HK_CHR_SULPHUR_DIOXIDE_DENSITY, read_sulphir_dioxide_density, NULL, true, chr_sulphur_dioxode_ptr);
    hk_setup_add_chr(HK_CHR_PM25_DENSITY, read_pm25_density, NULL, true, chr_pm25_ptr);
    hk_setup_add_chr(HK_CHR_PM10_DENSITY, read_pm10_density, NULL, true, chr_pm10_ptr);
    hk_setup_add_chr(HK_CHR_VOC_DENSITY, read_voc_density, NULL, true, chr_voc_ptr);
    hk_setup_add_chr(HK_CHR_STATUS_ACTIVE, read_active_status, NULL, true, chr_active_ptr);
    hk_setup_add_chr(HK_CHR_STATUS_TAMPERED, read_tampered_status, NULL, true, chr_tampered_ptr);
    hk_setup_add_chr(HK_CHR_STATUS_LOW_BATTERY, read_battery_status, NULL, true, chr_battery_ptr);
    hk_setup_finish();

    return ESP_OK;
}

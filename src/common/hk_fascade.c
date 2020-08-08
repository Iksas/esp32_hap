#include "../include/hk.h"
#include "../include/hk_fascade.h"

#include <stdbool.h>

void *hk_setup_add_switch(
    const char *name, 
    const char *manufacturer, 
    const char *model, 
    const char *serial_number, 
    const char *revision, 
    void (*identify)(), 
    esp_err_t (*read)(hk_mem* response), 
    esp_err_t (*write)(hk_mem* request))
{
    hk_setup_start();
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, identify);
    hk_setup_add_srv(HK_SRV_SWITCH, true, false);
    void *chr_handle = hk_setup_add_chr(HK_CHR_ON, read, write, true);
    hk_setup_finish();

    return chr_handle;
}

void *hk_setup_add_motion_sensor(
    const char *name, 
    const char *manufacturer, 
    const char *model, 
    const char *serial_number, 
    const char *revision, 
    void (*identify)(), 
    esp_err_t (*read)(hk_mem* response))
{
    hk_setup_start();
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, identify);
    hk_setup_add_srv(HK_SRV_MOTION_SENSOR, true, false);
    void *chr_handle = hk_setup_add_chr(HK_CHR_MOTION_DETECTED, read, NULL, true);
    hk_setup_finish();

    return chr_handle;
}
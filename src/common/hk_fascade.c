#include "../include/hk.h"
#include "../include/hk_fascade.h"

#include <stdbool.h>
void hk_setup_dummy_identify(){
}

void *hk_setup_add_switch(
    const char *name, 
    const char *manufacturer, 
    const char *model, 
    const char *serial_number, 
    const char *revision, 
    bool primary, 
    void (*identify)(), 
    esp_err_t (*read)(hk_mem* response), 
    esp_err_t (*write)(hk_mem* request))
{
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, identify);
    hk_setup_add_srv(HK_SRV_SWITCH, primary, false);
    return hk_setup_add_chr(HK_CHR_ON, read, write, true);
}

void *hk_setup_add_motion_sensor(
    const char *name, 
    const char *manufacturer, 
    const char *model, 
    const char *serial_number, 
    const char *revision, 
    bool primary,
    esp_err_t (*read)(hk_mem* response))
{
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, hk_setup_dummy_identify);
    hk_setup_add_srv(HK_SRV_MOTION_SENSOR, primary, false);
    return hk_setup_add_chr(HK_CHR_MOTION_DETECTED, read, NULL, true);
}
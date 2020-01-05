#include "include/homekit.h"
#include "include/homekit_fascade.h"

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
    void *(*read)(), 
    void (*write)(void *))
{
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, identify);
    hk_setup_add_service(HK_SRV_SWITCH, primary, false);
    return hk_setup_add_characteristic(HK_CHR_ON, read, write, true);
}

void *hk_setup_add_motion_sensor(
    const char *name, 
    const char *manufacturer, 
    const char *model, 
    const char *serial_number, 
    const char *revision, 
    bool primary,
    void *(*read)())
{
    hk_setup_add_accessory(name, manufacturer, model, serial_number, revision, hk_setup_dummy_identify);
    hk_setup_add_service(HK_SRV_MOTION_SENSOR, primary, false);
    return hk_setup_add_characteristic(HK_CHR_MOTION_DETECTED, read, NULL, true);
}
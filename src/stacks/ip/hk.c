#include "../../utils/hk_logging.h"
#include "../../include/hk.h"
#include "../../utils/hk_store.h"
#include "../../common/hk_accessory_id.h"
#include "../../common/hk_pairings_store.h"
#include "../../common/hk_code_store.h"
#include "../../common/hk_accessory_id.h"
#include "hk_server.h"
#include "hk_advertising.h"
#include "hk_chrs.h"
#include "hk_accessories_store.h"

void (*hk_identify_callback)();

esp_err_t hk_identify(hk_mem* request){
    if(hk_identify_callback != NULL){
        hk_identify_callback();
    }
    
    return ESP_OK;
}

esp_err_t hk_init(const char *name, const hk_categories_t category, const char *code)
{
    hk_code = code;
    hk_advertising_init(name, category, 2);
    hk_server_start();

    ESP_LOGD("homekit", "Inititialized.");

    return ESP_OK;
}

esp_err_t hk_setup_start()
{
    hk_store_init();
    hk_pairings_log_devices();
    
    return ESP_OK;
}

esp_err_t hk_setup_add_accessory(const char *name, const char *manufacturer, const char *model, const char *serial_number, const char *revision, void (*identify)())
{
    void *chr_ptr;
    hk_identify_callback = identify;
    hk_accessories_store_add_accessory();
    hk_accessories_store_add_srv(HK_SRV_ACCESSORY_INFORMATION, false, false);

    hk_accessories_store_add_chr_static_read(HK_CHR_NAME, (void *)name);
    hk_accessories_store_add_chr_static_read(HK_CHR_MANUFACTURER, (void *)manufacturer);
    hk_accessories_store_add_chr_static_read(HK_CHR_MODEL, (void *)model);
    hk_accessories_store_add_chr_static_read(HK_CHR_SERIAL_NUMBER, (void *)serial_number);
    hk_accessories_store_add_chr_static_read(HK_CHR_FIRMWARE_REVISION, (void *)revision);
    hk_accessories_store_add_chr(HK_CHR_IDENTIFY, NULL, hk_identify, false, &chr_ptr);
    
    return ESP_OK;
}

esp_err_t hk_setup_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden)
{
    hk_accessories_store_add_srv(srv_type, primary, hidden);
    
    return ESP_OK;
}

esp_err_t hk_setup_add_chr(hk_chr_types_t type, esp_err_t (*read)(hk_mem* response), esp_err_t (*write)(hk_mem* request), bool can_notify, void **chr_ptr)
{
    return hk_accessories_store_add_chr(type, read, write, can_notify, chr_ptr);
}

esp_err_t hk_setup_finish()
{
    hk_accessories_store_end_config();
    ESP_LOGD("homekit", "Set up.");
    
    return ESP_OK;
}

esp_err_t hk_reset()
{
    HK_LOGW("Resetting homekit for this device.");
    hk_accessory_id_reset();
    hk_pairings_store_remove_all();
    hk_advertising_reset();
    
    return ESP_OK;
}

esp_err_t hk_notify(void *chr)
{
    hk_chrs_notify(chr);
    
    return ESP_OK;
}
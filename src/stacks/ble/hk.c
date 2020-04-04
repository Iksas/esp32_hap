#include "../../utils/hk_logging.h"
#include "../../include/hk.h"
#include "../../utils/hk_store.h"
#include "../../common/hk_pairings_store.h"
#include "hk_nimble.h"
#include "hk_advertising.h"
#include "hk_gatt.h"
#include "hk_pairing_ble.h"

void (*hk_identify_callback)();

esp_err_t hk_read_protocol_information_version(hk_mem* response)
{
    HK_LOGE("hk_read_protocol_information_version");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t hk_identify(hk_mem* request, hk_mem* response){
    if(hk_identify_callback != NULL){
        hk_identify_callback();
    }
    
    return ESP_OK;
}

esp_err_t hk_read_chr_signature(hk_mem* response)
{
    HK_LOGE("hk_gatt_read_chr_signature");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t hk_write_chr_signature(hk_mem *request, hk_mem *response)
{
    HK_LOGE("hk_write_chr_signature");
    return ESP_ERR_NOT_SUPPORTED;
}

void hk_init(const char *name, const hk_categories_t category, const char *code)
{
    hk_store_code_set(code);
    hk_nimble_init();
    hk_advertising_init(name, category, 2);
    hk_gatt_start();
    hk_nimble_start();

    hk_pairings_log_devices();
    ESP_LOGD("homekit", "Started.");
}

void hk_setup_start()
{
    hk_store_init();
    hk_gatt_init();
}

void hk_setup_add_accessory(const char *name, const char *manufacturer, const char *model, const char *serial_number, const char *revision, void (*identify)())
{
    hk_identify_callback = identify;
    hk_store_configuration_set(2);
    hk_gatt_add_srv(HK_SRV_ACCESSORY_INFORMATION, false, false, false);

    hk_gatt_add_chr_static_read(HK_CHR_NAME, name);
    hk_gatt_add_chr_static_read(HK_CHR_MANUFACTURER, manufacturer);
    hk_gatt_add_chr_static_read(HK_CHR_MODEL, model);
    hk_gatt_add_chr_static_read(HK_CHR_SERIAL_NUMBER, serial_number);
    hk_gatt_add_chr_static_read(HK_CHR_FIRMWARE_REVISION, revision);
    hk_gatt_add_chr(HK_CHR_IDENTIFY, NULL, hk_identify, false, -1, -1); 

    hk_gatt_add_srv(HK_SRV_HAP_PROTOCOL_INFORMATION, false, false, true);
    hk_gatt_add_chr_static_read(HK_CHR_VERSION, "2.2.0");
    hk_gatt_add_chr(HK_CHR_SERVICE_SIGNATURE, hk_read_chr_signature, hk_write_chr_signature, false, -1, -1);

    hk_gatt_add_srv(HK_SRV_PARIRING, true, false, false);
    hk_gatt_add_chr(HK_CHR_PAIR_SETUP, hk_pairing_ble_read_pair_setup, hk_pairing_ble_write_pair_setup, false, -1, -1);
    hk_gatt_add_chr(HK_CHR_PAIR_VERIFY, hk_pairing_ble_read_pair_verify, hk_pairing_ble_write_pair_verify, false, -1, -1);
    hk_gatt_add_chr(HK_CHR_PAIRING_FEATURES, hk_pairing_ble_read_pairing_features, NULL, false, -1, -1);
    hk_gatt_add_chr(HK_CHR_PAIRING_PAIRINGS, hk_pairing_ble_read_pairing_pairings, hk_pairing_ble_write_pairing_pairings, false, -1, -1);
}

void hk_setup_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden)
{
    hk_gatt_add_srv(srv_type, primary, hidden, false);
}

void *hk_setup_add_chr(hk_chr_types_t type, esp_err_t (*read)(hk_mem* response), esp_err_t (*write)(hk_mem* request, hk_mem* response), bool can_notify)
{
    return hk_gatt_add_chr(type, read, write, can_notify, -1, -1);
}

void hk_setup_finish()
{
    hk_gatt_end_config();
    ESP_LOGD("homekit", "Set up.");
}

void hk_reset()
{
    HK_LOGW("Resetting homekit for this device.");
    hk_pairings_store_remove_all();
    hk_advertising_update_paired();
}

void hk_notify(void *chr)
{
    //hk_chrs_notify(chr);
}
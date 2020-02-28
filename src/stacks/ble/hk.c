#include "../../utils/hk_logging.h"
#include "../../include/hk.h"
#include "../../utils/hk_store.h"
#include "../../common/hk_pairings_store.h"
#include "hk_nimble.h"
#include "hk_gap.h"
#include "hk_gatt.h"
#include "hk_pairing_ble.h"

void (*hk_identify_callback)();

void *hk_gatt_read_accessory_information_srv_signature(size_t* response_length)
{
    HK_LOGE("hk_gatt_read_accessory_information_srv_signature");
    *response_length = 0;
    return NULL;
}

void *hk_gatt_read_chr_signature(size_t* response_length)
{
    HK_LOGE("hk_gatt_read_chr_signature");
    *response_length = 0;
    return NULL;
}

void hk_init(const char *name, const hk_categories_t category, const char *code)
{
    hk_store_code_set(code);
    hk_nimble_init();
    hk_gap_init(name, category, 2);
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

void* hk_identify(void* request, size_t request_size, size_t* response_size){
    if(hk_identify_callback != NULL){
        hk_identify_callback();
    }

    return NULL;
}

void hk_setup_add_accessory(const char *name, const char *manufacturer, const char *model, const char *serial_number, const char *revision, void (*identify)())
{
    hk_identify_callback = identify;
    hk_gatt_add_srv(HK_SRV_ACCESSORY_INFORMATION, false, false);

    hk_gatt_add_chr_static_read(HK_CHR_NAME, name);
    hk_gatt_add_chr_static_read(HK_CHR_MANUFACTURER, manufacturer);
    hk_gatt_add_chr_static_read(HK_CHR_MODEL, model);
    hk_gatt_add_chr_static_read(HK_CHR_SERIAL_NUMBER, serial_number);
    hk_gatt_add_chr_static_read(HK_CHR_FIRMWARE_REVISION, revision);
    hk_gatt_add_chr(HK_CHR_IDENTIFY, NULL, hk_identify, false, -1, -1); 

    hk_gatt_add_srv(HK_SRV_HAP_PROTOCOL_INFORMATION, true, false);
    hk_gatt_add_chr(HK_CHR_VERSION, hk_gatt_read_accessory_information_srv_signature, NULL, true, -1, 64);
    hk_gatt_add_chr(HK_CHR_SERVICE_SIGNATURE, hk_gatt_read_chr_signature, NULL, false, -1, -1);

    hk_gatt_add_srv(HK_SRV_PARIRING, true, false);
    hk_gatt_add_chr(HK_CHR_PAIR_SETUP, hk_pairing_ble_read_pair_setup, hk_pairing_ble_write_pair_setup, false, -1, -1);
    hk_gatt_add_chr(HK_CHR_PAIR_VERIFY, hk_pairing_ble_read_pair_verify, hk_pairing_ble_write_pair_verify, false, -1, -1);
    hk_gatt_add_chr(HK_CHR_PAIRING_FEATURES, hk_pairing_ble_read_pairing_features, NULL, false, -1, -1);
    hk_gatt_add_chr(HK_CHR_PAIRING_PAIRINGS, hk_pairing_ble_read_pairing_pairings, hk_pairing_ble_write_pairing_pairings, false, -1, -1);
}

void hk_setup_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden)
{
    hk_gatt_add_srv(srv_type, primary, hidden);
}

void *hk_setup_add_chr(hk_chr_types_t type, void *(*read)(size_t*), void* (*write)(void *, size_t, size_t*), bool can_notify)
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
    hk_store_is_paired_set(false);
    hk_advertising_update_paired(false);
}

void hk_notify(void *chr)
{
    //hk_chrs_notify(chr);
}
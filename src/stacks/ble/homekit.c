#include "../../utils/hk_logging.h"
#include "../../include/homekit.h"
#include "../../utils/hk_store.h"
#include "../../common/hk_pairings_store.h"
#include "hk_nimble.h"
#include "hk_gap.h"
#include "hk_gatt.h"

void *hk_gatt_read_accessory_information_service_signature()
{
    HK_LOGE("hk_gatt_read_accessory_information_service_signature");
    return 0;
}
void hk_gatt_write_accessory_information_service_signature()
{
    HK_LOGE("hk_gatt_write_accessory_information_service_signature");
    //return 0;
}

void *hk_gatt_read_pair_setup()
{
    HK_LOGD("hk_gatt_read_pair_setup");
    return 0;
}

void hk_gatt_write_pair_setup(void *new_value)
{
    HK_LOGD("hk_gatt_write_pair_setup");
}

void *hk_gatt_read_pair_verify()
{
    HK_LOGD("hk_gatt_read_pair_verify");
    return 0;
}

void hk_gatt_write_pair_verify(void *new_value)
{
    HK_LOGD("hk_gatt_write_pair_verify");
}

void *hk_gatt_read_pairing_features()
{
    HK_LOGD("hk_gatt_read_pairing_features");
    return 0;
}

void *hk_gatt_read_pairing_pairings()
{
    HK_LOGD("hk_gatt_read_pairing_pairings");
    return 0;
}

void hk_gatt_write_pairing_pairings(void *new_value)
{
    HK_LOGD("hk_gatt_write_pairing_pairings");
}

void *hk_gatt_read_accessory_information()
{
    HK_LOGD("hk_gatt_read_accessory_information");
    return 0;
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

void hk_setup_add_accessory(const char *name, const char *manufacturer, const char *model, const char *serial_number, const char *revision, void (*identify)())
{
    hk_gatt_add_service(HK_SRV_ACCESSORY_INFORMATION, false, false);

    hk_gatt_add_characteristic_static_read(HK_CHR_NAME, (void *)name);
    hk_gatt_add_characteristic_static_read(HK_CHR_MANUFACTURER, (void *)manufacturer);
    hk_gatt_add_characteristic_static_read(HK_CHR_MODEL, (void *)model);
    hk_gatt_add_characteristic_static_read(HK_CHR_SERIAL_NUMBER, (void *)serial_number);
    hk_gatt_add_characteristic_static_read(HK_CHR_FIRMWARE_REVISION, (void *)revision);
    hk_gatt_add_characteristic(HK_CHR_IDENTIFY, NULL, identify, false);
    //hk_gatt_add_characteristic(HK_CHR_SERVICE_SIGNATURE, hk_gatt_read_accessory_information_service_signature, hk_gatt_write_accessory_information_service_signature, false);

    hk_gatt_add_service(HK_SRV_HAP_PROTOCOL_INFORMATION, true, false);
    hk_gatt_add_characteristic(HK_CHR_VERSION, hk_gatt_read_accessory_information_service_signature, NULL, false);
    //hk_gatt_add_characteristic(HK_CHR_SERVICE_SIGNATURE, hk_gatt_read_accessory_information_service_signature, hk_gatt_write_accessory_information_service_signature, false);

    hk_gatt_add_service(HK_SRV_PARIRING, true, false);
    hk_gatt_add_characteristic(HK_CHR_PAIR_SETUP, hk_gatt_read_pair_setup, hk_gatt_write_pair_setup, false);
    hk_gatt_add_characteristic(HK_CHR_PAIR_VERIFY, hk_gatt_read_pair_verify, hk_gatt_write_pair_verify, false);
    hk_gatt_add_characteristic(HK_CHR_PAIRING_FEATURES, hk_gatt_read_pairing_features, NULL, false);
    hk_gatt_add_characteristic(HK_CHR_PAIRING_PAIRINGS, hk_gatt_read_pairing_pairings, hk_gatt_write_pairing_pairings, false);
    //hk_gatt_add_characteristic(HK_CHR_SERVICE_SIGNATURE, hk_gatt_read_accessory_information_service_signature, hk_gatt_write_accessory_information_service_signature, false);
}

void hk_setup_add_service(hk_service_types_t service_type, bool primary, bool hidden)
{
    hk_gatt_add_service(service_type, primary, hidden);
}

void *hk_setup_add_characteristic(hk_characteristic_types_t type, void *(*read)(), void (*write)(void *), bool can_notify)
{
    return hk_gatt_add_characteristic(type, read, write, can_notify);
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

void hk_notify(void *characteristic)
{
    //hk_characteristics_notify(characteristic);
}
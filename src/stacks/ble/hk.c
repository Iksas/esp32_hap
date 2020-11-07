#include "../../utils/hk_logging.h"
#include "../../include/hk.h"
#include "../../utils/hk_store.h"
#include "../../utils/hk_util.h"
#include "../../common/hk_accessory_id.h"
#include "../../common/hk_pairings_store.h"
#include "../../common/hk_global_state.h"
#include "hk_nimble.h"
#include "hk_gatt.h"
#include "hk_gap.h"
#include "hk_pairing_ble.h"

void (*hk_identify_callback)();

esp_err_t hk_read_protocol_information_version(hk_mem *response)
{
    HK_LOGE("hk_read_protocol_information_version");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t hk_identify(hk_mem *request)
{
    if (hk_identify_callback != NULL)
    {
        hk_identify_callback();
    }

    return ESP_OK;
}

esp_err_t hk_read_chr_signature(hk_mem *response)
{
    HK_LOGE("hk_gatt_read_chr_signature");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t hk_write_chr_signature(hk_mem *response)
{
    HK_LOGE("hk_write_chr_signature");
    return ESP_ERR_NOT_SUPPORTED;
}

void hk_init(const char *name, const hk_categories_t category, const char *code)
{
    hk_store_code_set(code);
    hk_global_state_init();
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

esp_err_t hk_setup_update_configuration_counter(const char *revision)
{
    hk_mem* revision_stored = hk_mem_init();

    esp_err_t ret = hk_store_blob_get(HK_REVISION_STORE_KEY, revision_stored);
    if (ret == ESP_ERR_NOT_FOUND) //ESP_ERR_NVS_NOT_FOUND
    {
        ret = ESP_OK;
        HK_LOGD("Found no revision number. Setting configuration counter to 1.");
        hk_mem_append_string(revision_stored, revision);
        RUN_AND_CHECK(ret, hk_store_blob_set, HK_REVISION_STORE_KEY, revision_stored);
        RUN_AND_CHECK(ret, hk_store_u8_set, HK_CONFIGURATION_STORE_KEY, 1);

        hk_mem_free(revision_stored);
        return ret;
    }
    else if (ret != ESP_OK)
    {
        HK_LOGE("Error updating configuration counter %x.", ret);
        hk_mem_free(revision_stored);
        return ret;
    }

    uint8_t configuration_counter = 0;
    if (!hk_mem_equal_str(revision_stored, revision))
    {
        ret = hk_store_u8_get(HK_CONFIGURATION_STORE_KEY, &configuration_counter);

        if (ret != ESP_OK)
        {
            HK_LOGW("Error getting configuration counter (%d). Setting counter to 0.", ret);
            configuration_counter = 0;
        }
        else
        {
            configuration_counter++;
        }

        ret = ESP_OK;
        HK_LOGI("Found new revision number (%s). Updating configuration counter to %d.", revision, configuration_counter);

        hk_mem_set(revision_stored, 0);
        hk_mem_append_string(revision_stored, revision);
        RUN_AND_CHECK(ret, hk_store_blob_set, HK_REVISION_STORE_KEY, revision_stored);
        RUN_AND_CHECK(ret, hk_store_u8_set, HK_CONFIGURATION_STORE_KEY, configuration_counter); 
    }

    hk_mem_free(revision_stored);

    return ret;
}

void hk_setup_add_accessory(const char *name, const char *manufacturer, const char *model, const char *serial_number, const char *revision, void (*identify)())
{
    hk_identify_callback = identify;
    hk_setup_update_configuration_counter(revision);

    hk_gatt_add_srv(HK_SRV_ACCESSORY_INFORMATION, false, false, false);

    hk_gatt_add_chr_static_read(HK_CHR_NAME, name);
    hk_gatt_add_chr_static_read(HK_CHR_MANUFACTURER, manufacturer);
    hk_gatt_add_chr_static_read(HK_CHR_MODEL, model);
    hk_gatt_add_chr_static_read(HK_CHR_SERIAL_NUMBER, serial_number);
    hk_gatt_add_chr_static_read(HK_CHR_FIRMWARE_REVISION, revision);
    hk_gatt_add_chr(HK_CHR_IDENTIFY, NULL, hk_identify, NULL, false, -1, -1);

    hk_gatt_add_srv(HK_SRV_HAP_PROTOCOL_INFORMATION, false, false, true);
    hk_gatt_add_chr_static_read(HK_CHR_VERSION, "2.2.0"); // version of specification
    hk_gatt_add_chr(HK_CHR_SERVICE_SIGNATURE, hk_read_chr_signature, hk_write_chr_signature, NULL, false, -1, -1);

    hk_gatt_add_srv(HK_SRV_PARIRING, true, false, false);
    hk_gatt_add_chr(HK_CHR_PAIR_SETUP, NULL, NULL, hk_pairing_ble_write_pair_setup, false, -1, -1);
    hk_gatt_add_chr(HK_CHR_PAIR_VERIFY, NULL, NULL, hk_pairing_ble_write_pair_verify, false, -1, -1);
    hk_gatt_add_chr(HK_CHR_PAIRING_FEATURES, hk_pairing_ble_read_pairing_features, NULL, NULL, false, -1, -1);
    hk_gatt_add_chr(HK_CHR_PAIRING_PAIRINGS, hk_pairing_ble_read_pairing_pairings, NULL, hk_pairing_ble_write_pairing_pairings, false, -1, -1);

}

void hk_setup_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden)
{
    hk_gatt_add_srv(srv_type, primary, hidden, false);
}

void *hk_setup_add_chr(hk_chr_types_t type, esp_err_t(*read)(hk_mem *response), esp_err_t(*write)(hk_mem *request), bool can_notify)
{
    return hk_gatt_add_chr(type, read, write, NULL, can_notify, -1, -1);
}

void hk_setup_finish()
{
    hk_gatt_end_config();
    ESP_LOGD("homekit", "Set up.");
}

void hk_reset()
{
    HK_LOGW("Resetting homekit for this device.");
    hk_accessory_id_reset();
    hk_global_state_reset();
    hk_pairings_store_remove_all();
}

void hk_notify(void *chr)
{
    hk_gatt_indicate(chr);
}
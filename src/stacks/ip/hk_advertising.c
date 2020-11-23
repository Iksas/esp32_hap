#include "hk_advertising.h"

#include <mdns.h>

#include "../../utils/hk_logging.h"
#include "../../common/hk_pairings_store.h"
#include "../../common/hk_accessory_id.h"
#include "../../utils/hk_util.h"
#include "../../include/hk_categories.h"

bool hk_advertising_is_running = false;

void hk_advertising_add_txt(const char *key, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);

    char value[128];
    int value_len = vsnprintf(value, sizeof(value), format, arg_ptr);

    va_end(arg_ptr);

    if (value_len && value_len < sizeof(value) - 1)
    {
        HK_LOGD("Adding service text: %s (%s)", key, value);
        ESP_ERROR_CHECK(mdns_service_txt_item_set("_hap", "_tcp", key, value));
    }
    else
    {
        HK_LOGE("Could not add service text: %s (%s)", key, value);
    }
}

void hk_advertising_init(const char *name, hk_categories_t category, size_t config_version)
{
    // spec 6.4 Discovery

    // Free in case of reconnecting to wifi
    mdns_free();

    // initialize mDNS
    ESP_ERROR_CHECK(mdns_init());
    // set mDNS hostname (required if you want to advertise services)
    ESP_ERROR_CHECK(mdns_hostname_set(name));
    // set mDNS instance name
    ESP_ERROR_CHECK(mdns_instance_name_set(name));
    ESP_ERROR_CHECK(mdns_service_add(name, "_hap", "_tcp", 5556, NULL, 0)); // accessory model name (required)

    // device ID (required), should be in format XX:XX:XX:XX:XX:XX, otherwise devices will ignore it
    hk_mem *accessory_id = hk_mem_init();
    hk_accessory_id_get_serialized(accessory_id);
    char *id_str = strndup(accessory_id->ptr, accessory_id->size);
    hk_advertising_add_txt("id", "%s", id_str);
    hk_mem_free(accessory_id);
    free(id_str);

    hk_advertising_add_txt("md", "%s", name);           // model name
    hk_advertising_add_txt("pv", "1.0");                // protocol version (required)
    hk_advertising_add_txt("c#", "%d", config_version); // current configuration number (required)
    hk_advertising_add_txt("s#", "1");                  // current state number (required)
    hk_advertising_add_txt("ff", "0");                  // see spec table 5.4 - its completely unclear what that is for.
    hk_advertising_add_txt("ci", "%d", category);       // accessory category identifier
    hk_advertising_update_paired();
    hk_advertising_is_running = true;
}

esp_err_t hk_advertising_update_paired()
{
    esp_err_t ret = ESP_OK;

    // if item is not paired, we need this flag. Otherwise not.
    if (hk_advertising_is_running)
    {
        ret = mdns_service_txt_item_remove("_hap", "_tcp", "sf");
    }

    bool paired = false;
    hk_pairings_store_has_pairing(&paired);
    if (!paired)
    {
        // spec Table 6.8 - status flags
        //   bit 1 - not paired
        //   bit 2 - not configured to join WiFi
        //   bit 3 - problem detected on accessory
        //   bits 4-8 - reserved
        hk_advertising_add_txt("sf", "1");
    }
    else
    {
        HK_LOGI("Not advertising for pairing, because we are coupled already.");
        //hk_advertising_add_txt("sf", "0");
    }

    return ret;
}

esp_err_t hk_advertising_reset()
{
    if (hk_advertising_is_running)
    {
        return hk_advertising_update_paired();
    }

    return ESP_OK;
}

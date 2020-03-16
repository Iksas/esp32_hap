#include "hk_advertising.h"

#include <mdns.h>

#include "../../utils/hk_logging.h"
#include "../../common/hk_pairings_store.h"
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
    //Free in case of reconnecting to wifi
    mdns_free(); 

    //initialize mDNS
    ESP_ERROR_CHECK(mdns_init());
    //set mDNS hostname (required if you want to advertise services)
    ESP_ERROR_CHECK(mdns_hostname_set(name));
    //set mDNS instance name
    ESP_ERROR_CHECK(mdns_instance_name_set(name));
    ESP_ERROR_CHECK(mdns_service_add(name, "_hap", "_tcp", 5556, NULL, 0)); // accessory model name (required)

    hk_advertising_add_txt("md", "%s", name);
    // protocol version (required)
    hk_advertising_add_txt("pv", "1.0");
    // device ID (required)
    // should be in format XX:XX:XX:XX:XX:XX, otherwise devices will ignore it
    hk_mem *id = hk_mem_create();
    hk_util_get_accessory_id_serialized(id);
    char* id_str = hk_mem_get_str(id);
    hk_advertising_add_txt("id", "%s", id_str);
    hk_mem_free(id);
    free(id_str);
    // current configuration number (required)
    hk_advertising_add_txt("c#", "%d", config_version);
    // current state number (required)
    hk_advertising_add_txt("s#", "1");
    // feature flags (required if non-zero)
    //   bit 0 - supports HAP pairing. required for all HomeKit accessories
    //   bits 1-7 - reserved
    // above is what the specification says
    // but we have to set this to zero for uncertified accessories (mfi)
    hk_advertising_add_txt("ff", "0");
    // accessory category identifier
    hk_advertising_add_txt("ci", "%d", category);
    hk_advertising_update_paired();
    hk_advertising_is_running = true;
}

void hk_advertising_update_paired()
{
    // if item is not paired, we need this flag. Otherwise not.
    if (hk_advertising_is_running)
    {
        ESP_ERROR_CHECK(mdns_service_txt_item_remove("_hap", "_tcp", "sf"));
    }

    bool paired = hk_pairings_store_has_pairing();
    if (!paired)
    {
        // status flags
        //   bit 0 - not paired
        //   bit 1 - not configured to join WiFi
        //   bit 2 - problem detected on accessory
        //   bits 3-7 - reserved
        hk_advertising_add_txt("sf", "1");
    }else{
        HK_LOGI("Not advertising for pairing, because we are coupled already.");
    }
}

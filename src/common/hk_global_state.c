#include "hk_global_state.h"

#include "../utils/hk_store.h"
#include "../utils/hk_logging.h"

#define HK_GLOBAL_STATE_STORE_KEY "hk_global_state"

void hk_global_state_init()
{
    uint16_t value = 0;
    esp_err_t err = hk_store_u16_get(HK_GLOBAL_STATE_STORE_KEY, &value);
    if(err == ESP_ERR_NOT_FOUND){
        hk_global_state_reset();
    }

    // set next to make controller read current value and generate broadcast key after restart
    hk_global_state_next();
}

uint16_t hk_global_state_get()
{
    uint16_t value = 0;
    hk_store_u16_get(HK_GLOBAL_STATE_STORE_KEY, &value);
    HK_LOGD("Globalstate requested: %d", value);
    return value;
}

void hk_global_state_next()
{
    uint16_t value = 0;
    hk_store_u16_get(HK_GLOBAL_STATE_STORE_KEY, &value);
    value++;

    if (value == 0)
    {
        value++;
    }
    hk_store_u16_set(HK_GLOBAL_STATE_STORE_KEY, value);
}

void hk_global_state_reset()
{
    hk_store_u16_set(HK_GLOBAL_STATE_STORE_KEY, 1);
}
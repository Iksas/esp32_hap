#include "hk_errors.h"

const char *hk_error_to_name(esp_err_t err){
    const char * ret = "";
    if (err == 0)
    {
        // doing nothing
    }  
    else if (err < 50000)
    {
        ret = esp_err_to_name(err);
    }
    else
    {
        switch (err)
        {
        case ESP_ERR_HK_TERMINATE:
            ret = "ESP_ERR_HK_TERMINATE";
            break;
        case ESP_ERR_HK_UNSUPPORTED_REQUEST:
            ret = "ESP_ERR_HK_UNSUPPORTED_REQUEST";
            break;
        case ESP_ERR_HK_UNKNOWN:
            ret = "ESP_ERR_HK_UNKNOWN";
            break;
        default:
            ret = "Unknown";
            break;
        }
    }

    return ret;
}
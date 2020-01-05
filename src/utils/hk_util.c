#include "hk_util.h"
#include "hk_logging.h"

#include <esp_system.h>

size_t hk_util_get_accessory_id(hk_mem *id)
{
    uint8_t mac[6] = {0, 0, 0, 0, 0, 0};

    esp_err_t ret = esp_efuse_mac_get_default(mac);
    if (ret)
    {
        HK_LOGEE(ret);
    }

    char accessory_id[18];
    sprintf(accessory_id, "%2X:%2X:%2X:%2X:%2X:%2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    hk_mem_append_buffer(id, accessory_id, 17);
    //hk_mem_append_buffer(id, "00:00:00:00:01:32", 17);

    return ret;
}

bool hk_util_string_ends_with(const char *str, const char *suffix)
{
    if (!str || !suffix)
    {
        return 0;
    }

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
    {
        return 0;
    }

    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}
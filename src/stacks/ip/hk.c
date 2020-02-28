#include "../../utils/hk_logging.h"
#include "../../include/hk.h"
#include "../../utils/hk_store.h"
#include "../../common/hk_pairings_store.h"
#include "hk_accessories_store.h"
#include "hk_server.h"
#include "hk_advertising.h"
#include "hk_chrs.h"

void hk_init(const char *name, const hk_categories_t category, const char *code)
{
    hk_store_code_set(code);
    hk_advertising_init(name, category, 2);
    hk_server_start();

    ESP_LOGD("homekit", "Inititialized.");
}

void hk_setup_start()
{
    hk_store_init();
    hk_pairings_log_devices();
}

void hk_setup_add_accessory(const char *name, const char *manufacturer, const char *model, const char *serial_number, const char *revision, void (*identify)())
{
    hk_accessories_store_add_accessory();
    hk_accessories_store_add_srv(HK_SRV_ACCESSORY_INFORMATION, false, false);

    hk_accessories_store_add_chr_static_read(HK_CHR_NAME, (void *)name);
    hk_accessories_store_add_chr_static_read(HK_CHR_MANUFACTURER, (void *)manufacturer);
    hk_accessories_store_add_chr_static_read(HK_CHR_MODEL, (void *)model);
    hk_accessories_store_add_chr_static_read(HK_CHR_SERIAL_NUMBER, (void *)serial_number);
    hk_accessories_store_add_chr_static_read(HK_CHR_FIRMWARE_REVISION, (void *)revision);
    hk_accessories_store_add_chr(HK_CHR_IDENTIFY, NULL, identify, false);
}

void hk_setup_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden)
{
    hk_accessories_store_add_srv(srv_type, primary, hidden);
}

void *hk_setup_add_chr(hk_chr_types_t type, void *(*read)(), void (*write)(void *, size_t), bool can_notify)
{
    return hk_accessories_store_add_chr(type, read, write, can_notify);
}

void hk_setup_finish()
{
    hk_accessories_store_end_config();
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
    hk_chrs_notify(chr);
}
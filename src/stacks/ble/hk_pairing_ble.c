#include "hk_pairing_ble.h"

#include "../../utils/hk_tlv.h"
#include "../../include/hk_mem.h"
#include "../../common/hk_pair_setup.h"
#include "../../common/hk_pair_verify.h"
#include "../../common/hk_pairings.h"
#include "../../common/hk_errors.h"
#include "hk_session_security.h"

const uint8_t hk_paring_ble_features = 0; //zero because non mfi certified
hk_mem *hk_pairing_ble_device_id = NULL; // todo: should be in connection

#include "../../utils/hk_logging.h"

esp_err_t hk_pairing_ble_read_pair_setup(hk_mem *response)
{
    HK_LOGE("hk_pairing_ble_read_pair_setup");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t hk_pairing_ble_write_pair_setup(hk_mem *request, hk_mem *response)
{
    esp_err_t res = ESP_OK;
    if (hk_pairing_ble_device_id == NULL)
    {
        hk_pairing_ble_device_id = hk_mem_create();
    }

    int rc = hk_pair_setup(request, response, hk_pairing_ble_device_id);
    if (rc != 0)
    {
        HK_LOGE("Error in pair setup: %d", res);
        res = ESP_ERR_HK_UNSUPPORTED_REQUEST;
    }

    return res;
}

esp_err_t hk_pairing_ble_read_pair_verify(hk_mem *response) //todo: remove
{
    HK_LOGE("hk_pairing_ble_read_pair_verify");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t hk_pairing_ble_write_pair_verify(hk_mem *request, hk_mem *response)
{
    bool is_encrypted;
    int res = hk_pair_verify(request, hk_session_security_keys_get(), response, &is_encrypted);
    if (res != 0)
    {
        HK_LOGE("Error in pair verify: %d", res);
    }
    
    hk_session_security_is_secured_set(is_encrypted);
    return ESP_OK;
}

esp_err_t hk_pairing_ble_read_pairing_features(hk_mem *response)
{
    hk_mem_append_buffer(response, (void *)&hk_paring_ble_features, sizeof(uint8_t));
    return ESP_OK;
}

esp_err_t hk_pairing_ble_read_pairing_pairings(hk_mem *response)
{
    HK_LOGE("hk_pairing_ble_read_pairing_pairings");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t hk_pairing_ble_write_pairing_pairings(hk_mem *request, hk_mem *response)
{
    bool kill_session = false;
    hk_pairings(request, response, &kill_session);
    HK_LOGI("Pairing removed. Killing session: %d", kill_session);
    if(kill_session){
        return ESP_ERR_HK_TERMINATE;
    }

    return ESP_OK;
}
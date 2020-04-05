#include "hk_pairing_ble.h"

#include "../../utils/hk_tlv.h"
#include "../../include/hk_mem.h"
#include "../../common/hk_pair_setup.h"
#include "../../common/hk_pair_verify.h"
#include "../../common/hk_pairings.h"
#include "../../common/hk_errors.h"
#include "hk_connection_security.h"

#include "../../utils/hk_logging.h"

esp_err_t hk_pairing_ble_write_pair_setup(hk_connection_t *connection, hk_mem *request, hk_mem *response)
{
    esp_err_t res = ESP_OK;

    int rc = hk_pair_setup(request, response, connection->device_id);
    if (rc != 0)
    {
        HK_LOGE("Error in pair setup: %d", res);
        res = ESP_ERR_HK_UNSUPPORTED_REQUEST;
    }

    return res;
}

esp_err_t hk_pairing_ble_write_pair_verify(hk_connection_t *connection, hk_mem *request, hk_mem *response)
{
    bool is_encrypted = false;
    int res = hk_pair_verify(request, connection->security_keys, response, &is_encrypted);
    if (res != 0)
    {
        HK_LOGE("Error in pair verify: %d", res);
    }

    connection->is_secure = is_encrypted;
    if (is_encrypted)
    {
        HK_LOGD("Connection now is secured.");
    }

    return ESP_OK;
}

esp_err_t hk_pairing_ble_read_pairing_features(hk_mem *response)
{
    const uint8_t hk_paring_ble_features = 0; //zero because non mfi certified
    hk_mem_append_buffer(response, (void *)&hk_paring_ble_features, sizeof(uint8_t));
    return ESP_OK;
}

esp_err_t hk_pairing_ble_read_pairing_pairings(hk_mem *response)
{
    HK_LOGE("hk_pairing_ble_read_pairing_pairings");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t hk_pairing_ble_write_pairing_pairings(hk_connection_t *connection, hk_mem *request, hk_mem *response)
{
    bool kill_chr = false;
    hk_pairings(request, response, &kill_chr);
    HK_LOGI("Pairing removed. Killing chr: %d", kill_chr);
    if (kill_chr)
    {
        return ESP_ERR_HK_TERMINATE;
    }

    return ESP_OK;
}
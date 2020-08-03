#include "hk_session_security.h"

#include "../../crypto/hk_chacha20poly1305.h"
#include "../../utils/hk_logging.h"

#define HK_AAD_SIZE 2
#define HK_AUTHTAG_SIZE 16 //16 = CHACHA20_POLY1305_AUTH_TAG_LENGTH
#define HK_MAX_DATA_SIZE 1024 - HK_AAD_SIZE - HK_AUTHTAG_SIZE

esp_err_t hk_connection_security_decrypt_frames(hk_encryption_data_t *encryption_data, hk_conn_key_store_t *keys, hk_mem *in, hk_mem *out)
{
    size_t offset = 0;
    while (offset < in->size)
    {
        char *encrypted = in->ptr + offset;
        size_t message_size = encrypted[0] + encrypted[1] * 256;
        char nonce[12] = {
            0,
        };
        nonce[4] = encryption_data->received_frame_count % 256;
        nonce[5] = encryption_data->received_frame_count++ / 256;
        char frame[message_size];

        esp_err_t ret = hk_chacha20poly1305_decrypt_buffer(
            keys->request_key, nonce, encrypted, HK_AAD_SIZE, encrypted + HK_AAD_SIZE, frame, message_size);

        if (ret)
        {
            return ret;
        }
        else
        {
            hk_mem_append_buffer(out, frame, message_size);
        }

        offset += message_size + HK_AAD_SIZE + HK_AUTHTAG_SIZE;
    }

    return ESP_OK;
}

esp_err_t hk_connection_security_encrypt_frames(hk_encryption_data_t *encryption_data, hk_conn_key_store_t *keys, hk_mem *in, 
    esp_err_t (*callback)(hk_mem *frame_data, void *args), void* args)
{
    char nonce[12] = {
        0,
    };
    size_t pending_size = in->size;
    char *pending = in->ptr;
    while (pending_size > 0)
    {
        size_t chunk_size = pending_size < HK_MAX_DATA_SIZE ? pending_size : HK_MAX_DATA_SIZE;
        pending_size -= chunk_size;
        size_t encrypted_size = HK_AAD_SIZE + chunk_size + HK_AUTHTAG_SIZE;
        char encrypted[encrypted_size];
        encrypted[0] = chunk_size % 256;
        encrypted[1] = chunk_size / 256;

        nonce[4] = encryption_data->sent_frame_count % 256;
        nonce[5] = encryption_data->sent_frame_count++ / 256;

        esp_err_t ret = hk_chacha20poly1305_encrypt_buffer(keys->response_key, nonce, encrypted, HK_AAD_SIZE,
                                                           pending, encrypted + HK_AAD_SIZE, chunk_size);
        if (ret < 0)
        {
            return ret;
        }
        hk_mem* data_to_send = hk_mem_init(); // is disposed by server, after it was sent
        hk_mem_append_buffer(data_to_send, encrypted, encrypted_size);
        ret = callback(data_to_send, args);
        if (ret < 0)
        {
            return ret;
        }

        pending += chunk_size;
    }

    return ESP_OK;
}

hk_encryption_data_t *hk_encryption_data_init()
{
    hk_encryption_data_t *data = (hk_encryption_data_t *)malloc(sizeof(hk_encryption_data_t));

    data->sent_frame_count = 0;
    data->received_frame_count = 0;

    return data;
}

void hk_encryption_data_free(hk_encryption_data_t *data)
{
    free(data);
}
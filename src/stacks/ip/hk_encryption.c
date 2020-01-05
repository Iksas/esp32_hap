#include "hk_encryption.h"

#include "../../crypto/hk_chacha20poly1305.h"
#include "../../utils/hk_logging.h"
#include "hk_com.h"

#define HK_AAD_SIZE 2
#define HK_AUTHTAG_SIZE 16 //16 = CHACHA20_POLY1305_AUTH_TAG_LENGTH
#define HK_MAX_DATA_SIZE 1024 - HK_AAD_SIZE - HK_AUTHTAG_SIZE

esp_err_t hk_encryption_preprocessor(hk_session_t *session, hk_mem *in, hk_mem *out)
{
    if (session->is_encrypted)
    {
        size_t offset = 0;
        while (offset < in->size)
        {
            char *encrypted = in->ptr + offset;
            size_t message_size = encrypted[0] + encrypted[1] * 256;
            char nonce[12] = {
                0,
            };
            nonce[4] = session->received_frame_count % 256;
            nonce[5] = session->received_frame_count++ / 256;

            char frame[message_size];
            esp_err_t ret = hk_chacha20poly1305_decrypt_buffer(
                session->keys->request_key, nonce, encrypted, HK_AAD_SIZE, encrypted + HK_AAD_SIZE, frame, message_size);
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
        
        //hk_mem_log("Decrcypted", out);
    }
    else
    {
        hk_mem_append(out, in);
    }

    return ESP_OK;
}

esp_err_t hk_encryption_postprocessor(hk_session_t *session, hk_mem *in)
{
    if (session->is_encrypted)
    {
        //hk_mem_log("Encrypting", in);
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

            nonce[4] = session->sent_frame_count % 256;
            nonce[5] = session->sent_frame_count++ / 256;

            esp_err_t ret = hk_chacha20poly1305_encrypt_buffer(session->keys->response_key, nonce, encrypted, HK_AAD_SIZE,
                                                               pending, encrypted + HK_AAD_SIZE, chunk_size);
            if (ret < 0)
            {
                return ret;
            }

            hk_mem *out = hk_mem_create(); //freed by hk_com after sending
            hk_mem_append_buffer(out, encrypted, encrypted_size);
            hk_com_send_data(session, out);

            pending += chunk_size;
        }
    }
    else
    {
        hk_mem *out = hk_mem_create(); //freed by hk_com after sending
        hk_mem_append(out, in);
        hk_com_send_data(session, out);
    }

    return ESP_OK;
}
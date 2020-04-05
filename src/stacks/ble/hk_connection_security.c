#include "hk_connection_security.h"

#include "../../crypto/hk_chacha20poly1305.h"
#include "../../utils/hk_logging.h"

#define HK_AUTHTAG_SIZE 16 //16 = CHACHA20_POLY1305_AUTH_TAG_LENGTH

esp_err_t hk_connection_security_decrypt(hk_connection_t *connection, hk_mem *in, hk_mem *out)
{
    char nonce[12] = {
        0,
    };
    nonce[4] = connection->received_frame_count % 256;
    nonce[5] = connection->received_frame_count++ / 256;
    size_t message_size = in->size - HK_AUTHTAG_SIZE;
    hk_mem_set(out, message_size);
    esp_err_t ret = hk_chacha20poly1305_decrypt_buffer(
        connection->security_keys->request_key, nonce,
        NULL, 0,
        (char *)in->ptr, (char *)out->ptr, message_size);

    return ret;
}

esp_err_t hk_connection_security_encrypt(hk_connection_t *connection, hk_mem *in, hk_mem *out)
{
    hk_mem_set(out, in->size + HK_AUTHTAG_SIZE);

    char nonce[12] = {
        0,
    };
    nonce[4] = connection->sent_frame_count % 256;
    nonce[5] = connection->sent_frame_count++ / 256;

    esp_err_t ret = hk_chacha20poly1305_encrypt_buffer(
        connection->security_keys->response_key, nonce,
        NULL, 0,
        in->ptr, out->ptr, in->size);

    return ret;
}
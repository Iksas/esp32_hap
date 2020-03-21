#include "hk_session_security.h"

#include "../../crypto/hk_chacha20poly1305.h"
#include "../../utils/hk_logging.h"

#define HK_AUTHTAG_SIZE 16 //16 = CHACHA20_POLY1305_AUTH_TAG_LENGTH

hk_pair_verify_keys_t *hk_session_security_keys = NULL;
bool hk_session_security_is_secured = false;
int hk_session_security_received_frame_count = 0;
int hk_session_security_sent_frame_count = 0;

void hk_session_security_init_or_reset()
{
    if (hk_session_security_keys == NULL)
    {
        HK_LOGD("Initializing keys");
        hk_session_security_keys = hk_pair_verify_keys_init();
    }

    hk_session_security_is_secured = false;
    hk_session_security_received_frame_count = 0;
    hk_session_security_sent_frame_count = 0;
}

hk_pair_verify_keys_t *hk_session_security_keys_get()
{
    return hk_session_security_keys;
}

bool hk_session_security_is_secured_get()
{
    return hk_session_security_is_secured;
}

void hk_session_security_is_secured_set(bool is_encrypted)
{
    hk_session_security_is_secured = is_encrypted;
}

esp_err_t hk_session_security_decrypt(hk_mem *in, hk_mem *out)
{
    char nonce[12] = {
        0,
    };
    nonce[4] = hk_session_security_received_frame_count % 256;
    nonce[5] = hk_session_security_received_frame_count++ / 256;
    size_t message_size = in->size - HK_AUTHTAG_SIZE;
    hk_mem_set(out, message_size);
    esp_err_t ret = hk_chacha20poly1305_decrypt_buffer(
        hk_session_security_keys->request_key, nonce,
        NULL, 0,
        (char *)in->ptr, (char *)out->ptr, message_size);

    return ret;
}

esp_err_t hk_session_security_encrypt(hk_mem *in, hk_mem *out)
{
    hk_mem_set(out, in->size + HK_AUTHTAG_SIZE);

    char nonce[12] = {
        0,
    };
    nonce[4] = hk_session_security_sent_frame_count % 256;
    nonce[5] = hk_session_security_sent_frame_count++ / 256;

    esp_err_t ret = hk_chacha20poly1305_encrypt_buffer(
        hk_session_security_keys->response_key, nonce,
        NULL, 0,
        in->ptr, out->ptr, in->size);

    return ret;
}
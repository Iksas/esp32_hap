#include "hk_pair_verify.h"

#include "../crypto/hk_curve25519.h"
#include "../crypto/hk_hkdf.h"
#include "../crypto/hk_chacha20poly1305.h"
#include "../crypto/hk_ed25519.h"
#include "../utils/hk_logging.h"
#include "../utils/hk_tlv.h"
#include "../utils/hk_util.h"
#include "../utils/hk_store.h"
#include "../utils/hk_ll.h"

#include "hk_accessory_id.h"
#include "hk_pairings_store.h"
#include "hk_pair_tlvs.h"

typedef struct
{
    hk_mem *id;
    hk_mem *shared_secret;
} hk_pair_verify_session_t;

hk_pair_verify_session_t *hk_pair_verify_sessions = NULL;

esp_err_t hk_pair_verify_create_session(hk_conn_key_store_t *keys)
{
    esp_err_t ret = ESP_OK;

    hk_mem *session_id = hk_mem_init();
    ret = hk_hkdf_with_given_size(keys->shared_secret, session_id, 8, HK_HKDF_PAIR_VERIFY_RESUME_SALT, HK_HKDF_PAIR_VERIFY_RESUME_INFO);

    if (!ret && hk_pair_verify_sessions != NULL && hk_ll_count(hk_pair_verify_sessions) >= 8)
    {
        HK_LOGE("To many sessions and automatic deletion strategy is still to be implemented.");
    }

    if (!ret)
    {
        hk_pair_verify_sessions = hk_ll_init(hk_pair_verify_sessions);
        hk_pair_verify_sessions->id = hk_mem_init();
        hk_pair_verify_sessions->shared_secret = hk_mem_init();
        hk_mem_append(hk_pair_verify_sessions->id, session_id);
        hk_mem_append(hk_pair_verify_sessions->shared_secret, keys->shared_secret);
    }

    hk_mem_free(session_id);

    return ret;
}

esp_err_t hk_create_session_security(hk_conn_key_store_t *keys)
{
    esp_err_t ret = ESP_OK;

    ret = hk_hkdf(keys->shared_secret, keys->response_key, HK_HKDF_CONTROL_READ_SALT, HK_HKDF_CONTROL_READ_INFO);

    if (!ret)
    {
        ret = hk_hkdf(keys->shared_secret, keys->request_key, HK_HKDF_CONTROL_WRITE_SALT, HK_HKDF_CONTROL_WRITE_INFO);
    }

    return ret;
}

esp_err_t hk_pair_verify_start(hk_conn_key_store_t *keys, hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr)
{
    HK_LOGD("Now running pair verify start.");
    esp_err_t ret = ESP_OK;
    hk_ed25519_key_t *accessory_long_term_key = hk_ed25519_init();
    hk_curve25519_key_t *accessory_session_key = hk_curve25519_init();
    hk_curve25519_key_t *device_session_key = hk_curve25519_init();
    hk_mem *accessory_info = hk_mem_init();
    hk_mem *accessory_id = hk_mem_init();
    hk_mem *accessory_signature = hk_mem_init();
    hk_mem *accessory_public_key = hk_mem_init();
    hk_mem *accessory_private_key = hk_mem_init();
    hk_mem *sub_result = hk_mem_init();
    hk_mem *encrypted = hk_mem_init();
    hk_tlv_t *response_sub_tlvs = NULL;
    hk_tlv_t *response_tlvs = NULL;

    hk_conn_key_store_reset(keys);
    keys->session_key = hk_mem_init();
    keys->accessory_session_key_public = hk_mem_init();
    keys->device_session_key_public = hk_mem_init();

    if (hk_store_keys_can_get())
    {
        hk_store_key_pub_get(accessory_public_key);
        hk_store_key_priv_get(accessory_private_key);
    }
    else
    {
        HK_LOGE("Cannot get key from store for pairing.");
        ret = ESP_ERR_NOT_FOUND;
    }

    if (accessory_public_key->size < 1 || accessory_private_key->size < 1)
    {
        HK_LOGE("Verfiy was called, but accessory keys are not present.");
        ret = ESP_ERR_NOT_FOUND;
    }

    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, request_tlvs, HK_PAIR_TLV_PUBLICKEY, keys->device_session_key_public);
    RUN_AND_CHECK(ret, hk_curve25519_update_from_public_key, keys->device_session_key_public, device_session_key);
    RUN_AND_CHECK(ret, hk_curve25519_update_from_random, accessory_session_key);
    RUN_AND_CHECK(ret, hk_curve25519_calculate_shared_secret, accessory_session_key, device_session_key, keys->shared_secret);
    RUN_AND_CHECK(ret, hk_curve25519_export_public_key, accessory_session_key, keys->accessory_session_key_public);
    RUN_AND_CHECK(ret, hk_ed25519_update_from_random_keys, accessory_long_term_key, accessory_private_key, accessory_public_key);
    RUN_AND_CHECK(ret, hk_accessory_id_get_serialized, accessory_id);

    if (!ret)
    {
        hk_mem_append(accessory_info, keys->accessory_session_key_public);
        hk_mem_append(accessory_info, accessory_id);
        hk_mem_append(accessory_info, keys->device_session_key_public);
    }

    RUN_AND_CHECK(ret, hk_ed25519_sign, accessory_long_term_key, accessory_info, accessory_signature);

    if (!ret)
    {
        response_sub_tlvs = hk_tlv_add(response_sub_tlvs, HK_PAIR_TLV_IDENTIFIER, accessory_id);
        response_sub_tlvs = hk_tlv_add(response_sub_tlvs, HK_PAIR_TLV_SIGNATURE, accessory_signature);

        hk_tlv_serialize(response_sub_tlvs, sub_result);
    }

    RUN_AND_CHECK(ret, hk_hkdf, keys->shared_secret, keys->session_key, HK_HKDF_PAIR_VERIFY_ENCRYPT_SALT, HK_HKDF_PAIR_VERIFY_ENCRYPT_INFO);
    RUN_AND_CHECK(ret, hk_chacha20poly1305_encrypt, keys->session_key, HK_CHACHA_VERIFY_MSG2, sub_result, encrypted);
    RUN_AND_CHECK(ret, hk_create_session_security, keys);

    response_tlvs = hk_tlv_add_uint8(response_tlvs, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M2);
    if (!ret)
    {
        response_tlvs = hk_tlv_add(response_tlvs, HK_PAIR_TLV_PUBLICKEY, keys->accessory_session_key_public); // there is an error in the specification, dont use the srp proof
        response_tlvs = hk_tlv_add(response_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted);
    }
    else
    {
        response_tlvs = hk_tlv_add_uint8(response_tlvs, HK_PAIR_TLV_ERROR, HK_PAIR_TLV_ERROR_AUTHENTICATION);
    }

    *response_tlvs_ptr = response_tlvs;

    hk_tlv_free(response_sub_tlvs);
    hk_mem_free(accessory_info);
    hk_mem_free(accessory_id);
    hk_mem_free(accessory_signature);
    hk_mem_free(accessory_public_key);
    hk_mem_free(accessory_private_key);
    hk_mem_free(sub_result);
    hk_mem_free(encrypted);
    hk_ed25519_free(accessory_long_term_key);
    hk_curve25519_free(accessory_session_key);
    hk_curve25519_free(device_session_key);

    return ret;
}

esp_err_t hk_pair_verify_finish(hk_conn_key_store_t *keys, hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr)
{
    HK_LOGD("Now running pair verify finish.");
    hk_ed25519_key_t *device_long_term_key = hk_ed25519_init();
    hk_mem *device_long_term_key_public = hk_mem_init();
    hk_mem *device_info = hk_mem_init();
    hk_mem *device_id = hk_mem_init();

    hk_mem *encrypted_data = hk_mem_init();
    hk_mem *decrypted_data = hk_mem_init();
    hk_mem *device_signature = hk_mem_init();
    hk_tlv_t *request_tlvs_decrypted = NULL;
    hk_tlv_t *response_tlvs = NULL;

    esp_err_t ret = hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);

    RUN_AND_CHECK(ret, hk_chacha20poly1305_decrypt, keys->session_key, HK_CHACHA_VERIFY_MSG3, encrypted_data, decrypted_data);

    if (!ret)
    {
        request_tlvs_decrypted = hk_tlv_deserialize(decrypted_data);
    }

    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, request_tlvs_decrypted, HK_PAIR_TLV_IDENTIFIER, device_id);
    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, request_tlvs_decrypted, HK_PAIR_TLV_SIGNATURE, device_signature);
    RUN_AND_CHECK(ret, hk_pairings_store_ltpk_get, device_id, device_long_term_key_public);
    RUN_AND_CHECK(ret, hk_ed25519_update_from_random_from_public_key, device_long_term_key, device_long_term_key_public);

    if (!ret)
    {
        hk_mem_append(device_info, keys->device_session_key_public);
        hk_mem_append(device_info, device_id);
        hk_mem_append(device_info, keys->accessory_session_key_public);
    }

    RUN_AND_CHECK(ret, hk_ed25519_verify, device_long_term_key, device_signature, device_info);

    response_tlvs = hk_tlv_add_uint8(response_tlvs, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M4);
    if (ret)
    {
        response_tlvs = hk_tlv_add_uint8(response_tlvs, HK_PAIR_TLV_ERROR, HK_PAIR_TLV_ERROR_AUTHENTICATION);
    }
    else
    {
        hk_pair_verify_create_session(keys);
    }

    *response_tlvs_ptr = response_tlvs;

    hk_ed25519_free(device_long_term_key);
    hk_tlv_free(request_tlvs_decrypted);
    hk_mem_free(device_info);
    hk_mem_free(encrypted_data);
    hk_mem_free(decrypted_data);
    hk_mem_free(device_signature);
    hk_mem_free(device_long_term_key_public);
    hk_mem_free(device_id);

    hk_mem_free(keys->session_key);
    hk_mem_free(keys->accessory_session_key_public);
    hk_mem_free(keys->device_session_key_public);

    return ret;
}

esp_err_t hk_pair_verify_resume(hk_conn_key_store_t *keys, hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr)
{
    hk_mem *session_id = hk_mem_init();
    hk_mem *encryption_key = hk_mem_init();
    hk_mem *encrypted_data = hk_mem_init();
    hk_mem *decrypted_data = hk_mem_init();
    hk_mem_append_string(decrypted_data, "01234567890123456789012345678901");
    hk_mem *device_session_key_public = hk_mem_init();
    hk_mem *salt = hk_mem_init();
    hk_tlv_t *response_tlvs = NULL;
    esp_err_t ret = ESP_OK;

    ret = hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_SESSIONID, session_id);

    hk_pair_verify_session_t *session = NULL;
    hk_ll_foreach(hk_pair_verify_sessions, s)
    {
        if (hk_mem_equal(s->id, session_id))
        {
            session = s;
            //todo: break;
        }
    }

    if (session == NULL)
    {
        HK_LOGD("No session for resume found.");
        ret = ESP_ERR_NOT_FOUND;
    }

    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, request_tlvs, HK_PAIR_TLV_PUBLICKEY, device_session_key_public);
    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, request_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);

    if (!ret)
    {
        hk_mem_append(salt, device_session_key_public);
        hk_mem_append(salt, session_id);
    }
    RUN_AND_CHECK(ret, hk_hkdf_with_external_salt, session->shared_secret, encryption_key, salt, HK_HKDF_PAIR_RESUME_REQUEST_INFO);
    RUN_AND_CHECK(ret, hk_chacha20poly1305_verify_auth_tag, encryption_key, HK_CHACHA_RESUME_MSG1, encrypted_data);

    if (!ret)
    {
        hk_mem_set(session->id, 0);
        uint32_t random_number = esp_random();
        hk_mem_append_buffer(session->id, &random_number, sizeof(uint32_t));
        random_number = esp_random();
        hk_mem_append_buffer(session->id, &random_number, sizeof(uint32_t));
    }

    if (!ret)
    {
        hk_mem_set(salt, 0);
        hk_mem_set(encryption_key, 0);
        hk_mem_append(salt, device_session_key_public);
        hk_mem_append(salt, session->id);
    }
    RUN_AND_CHECK(ret, hk_hkdf_with_external_salt, session->shared_secret, encryption_key, salt, HK_HKDF_PAIR_RESUME_RESPONSE_INFO);

    if (!ret)
    {
        hk_mem_set(encrypted_data, 0);
        hk_conn_key_store_reset(keys);
    }

    RUN_AND_CHECK(ret, hk_chacha20poly1305_caluclate_auth_tag_without_message, encryption_key, HK_CHACHA_RESUME_MSG2, encrypted_data);
    RUN_AND_CHECK(ret, hk_hkdf_with_external_salt, session->shared_secret, keys->shared_secret, salt, HK_HKDF_PAIR_RESUME_SHARED_SECRET_INFO);

    if (!ret)
    {
        hk_mem_set(session->shared_secret, 0);
        hk_mem_append(session->shared_secret, keys->shared_secret);
        hk_create_session_security(keys);
    }

    response_tlvs = hk_tlv_add_uint8(response_tlvs, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M2);
    if (!ret)
    {
        response_tlvs = hk_tlv_add_uint8(response_tlvs, HK_PAIR_TLV_METHOD, HK_PAIR_TLV_METHOD_RESUME);
        response_tlvs = hk_tlv_add_mem(response_tlvs, HK_PAIR_TLV_SESSIONID, session->id);
        response_tlvs = hk_tlv_add_mem(response_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);
    }
    else
    {
        // todo: send error if session was found, but encryption failed
        if (session != NULL)
        {
            hk_mem_free(session->id);
            hk_mem_free(session->shared_secret);
            hk_ll_remove(hk_pair_verify_sessions, session);
        }
    }

    *response_tlvs_ptr = response_tlvs;

    hk_mem_free(session_id);
    hk_mem_free(encryption_key);
    hk_mem_free(encrypted_data);
    hk_mem_free(decrypted_data);
    hk_mem_free(device_session_key_public);
    hk_mem_free(salt);

    return ret;
}

int hk_pair_verify(hk_mem *request, hk_mem *result, hk_conn_key_store_t *keys, bool *is_session_encrypted)
{
    esp_err_t res = ESP_OK;
    hk_tlv_t *request_tlvs = hk_tlv_deserialize(request);
    hk_tlv_t *response_tlvs = NULL;
    hk_tlv_t *state_tlv = hk_tlv_get_tlv_by_type(request_tlvs, HK_PAIR_TLV_STATE);

    if (state_tlv == NULL)
    {
        HK_LOGE("Could not find tlv with type state.");
        res = ESP_ERR_HK_UNSUPPORTED_REQUEST;
    }
    else
    {
        switch (*state_tlv->value)
        {
        case HK_PAIR_TLV_STATE_M1:
        {
            hk_tlv_t *method_tlv = hk_tlv_get_tlv_by_type(request_tlvs, HK_PAIR_TLV_METHOD);
            if (method_tlv != NULL)
            {
                if (*method_tlv->value == HK_PAIR_TLV_METHOD_RESUME)
                {
                    res = hk_pair_verify_resume(keys, request_tlvs, &response_tlvs);
                    if (res == ESP_OK)
                    {
                        *is_session_encrypted = true;
                    }
                    else
                    {
                        res = ESP_OK;
                        res = hk_pair_verify_start(keys, request_tlvs, &response_tlvs);
                    }
                }
            }
            else
            {
                res = hk_pair_verify_start(keys, request_tlvs, &response_tlvs);
            }
            break;
        }
        case HK_PAIR_TLV_STATE_M3:
        {
            res = hk_pair_verify_finish(keys, request_tlvs, &response_tlvs);

            if (res == ESP_OK)
            {
                *is_session_encrypted = true;
            }
            break;
        }
        default:
            HK_LOGE("Unexpected value in tlv in pair setup: %d", *state_tlv->value);
            res = ESP_ERR_HK_UNSUPPORTED_REQUEST;
        }
    }

    hk_tlv_serialize(response_tlvs, result);

    hk_tlv_free(request_tlvs);
    hk_tlv_free(response_tlvs);

    return res;
}
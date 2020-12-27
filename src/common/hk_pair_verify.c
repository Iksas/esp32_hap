#include "hk_pair_verify.h"

#include "../crypto/hk_curve25519.h"
#include "../crypto/hk_hkdf.h"
#include "../crypto/hk_chacha20poly1305.h"
#include "../crypto/hk_ed25519.h"
#include "../utils/hk_logging.h"
#include "../utils/hk_tlv.h"
#include "../utils/hk_util.h"
#include "../utils/hk_ll.h"

#include "hk_accessory_id.h"
#include "hk_pairings_store.h"
#include "hk_pair_tlvs.h"
#include "hk_key_store.h"

typedef struct
{
    hk_mem *id;
    hk_mem *accessory_shared_secret;
} hk_pair_verify_session_t;

hk_pair_verify_session_t *hk_pair_verify_sessions = NULL;

esp_err_t hk_pair_verify_create_session(hk_conn_key_store_t *keys)
{
    esp_err_t ret = ESP_OK;

    hk_mem *session_id = hk_mem_init();
    ret = hk_hkdf_with_given_size(keys->accessory_shared_secret, session_id, 8, HK_HKDF_PAIR_VERIFY_RESUME_SALT, HK_HKDF_PAIR_VERIFY_RESUME_INFO);

    if (!ret && hk_pair_verify_sessions != NULL && hk_ll_count(hk_pair_verify_sessions) >= 8)
    {
        HK_LOGE("To many sessions and automatic deletion strategy is still to be implemented.");
    }

    if (!ret)
    {
        HK_LOGV("Currently we have %d sessions. Adding new one.", hk_ll_count(hk_pair_verify_sessions));
        hk_pair_verify_sessions = hk_ll_init(hk_pair_verify_sessions);
        hk_pair_verify_sessions->id = hk_mem_init();
        hk_pair_verify_sessions->accessory_shared_secret = hk_mem_init();
        hk_mem_append(hk_pair_verify_sessions->id, session_id);
        hk_mem_append(hk_pair_verify_sessions->accessory_shared_secret, keys->accessory_shared_secret);
    }

    hk_mem_free(session_id);

    return ret;
}

static esp_err_t hk_pair_verify_create_session_security(hk_conn_key_store_t *keys)
{
    esp_err_t ret = ESP_OK;

    ret = hk_hkdf(keys->accessory_shared_secret, keys->response_key, HK_HKDF_CONTROL_READ_SALT, HK_HKDF_CONTROL_READ_INFO);

    if (!ret)
    {
        ret = hk_hkdf(keys->accessory_shared_secret, keys->request_key, HK_HKDF_CONTROL_WRITE_SALT, HK_HKDF_CONTROL_WRITE_INFO);
    }

    return ret;
}

esp_err_t hk_pair_verify_start(hk_conn_key_store_t *keys, hk_tlv_t *request_tlvs, hk_mem *response)
{
    // see spec chapter 5.7.2
    HK_LOGV("Now running pair verify start.");
    esp_err_t ret = ESP_OK;
    hk_ed25519_key_t *accessory_long_term_key = hk_ed25519_init();
    hk_curve25519_key_t *accessory_curve_key_pair = hk_curve25519_init();
    hk_curve25519_key_t *device_curve_key_pair = hk_curve25519_init();
    hk_mem *accessory_info = hk_mem_init();
    hk_mem *accessory_id = hk_mem_init();
    hk_mem *accessory_signature = hk_mem_init();
    hk_mem *accessory_public_key = hk_mem_init();
    hk_mem *accessory_private_key = hk_mem_init();
    hk_mem *sub_result = hk_mem_init();
    hk_mem *encrypted = hk_mem_init();
    hk_tlv_t *tlv_data_response_sub = NULL;
    hk_tlv_t *tlv_data_response = NULL;

    hk_conn_key_store_reset(keys);

    RUN_AND_CHECK(ret, hk_key_store_pub_get, accessory_public_key);
    RUN_AND_CHECK(ret, hk_key_store_priv_get, accessory_private_key);

    // spec 5.7.2.1
    RUN_AND_CHECK(ret, hk_curve25519_update_from_random, accessory_curve_key_pair);

    // spec 5.7.2.2
    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, request_tlvs, HK_PAIR_TLV_PUBLICKEY, keys->device_session_key_public);
    RUN_AND_CHECK(ret, hk_curve25519_update_from_public_key, keys->device_session_key_public, device_curve_key_pair);
    RUN_AND_CHECK(ret, hk_curve25519_calculate_shared_secret, accessory_curve_key_pair, device_curve_key_pair, keys->accessory_shared_secret);

    // spec 5.7.2.3
    RUN_AND_CHECK(ret, hk_curve25519_export_public_key, accessory_curve_key_pair, keys->accessory_session_key_public);
    RUN_AND_CHECK(ret, hk_accessory_id_get_serialized, accessory_id);
    if (!ret)
    {
        hk_mem_append(accessory_info, keys->accessory_session_key_public);
        hk_mem_append(accessory_info, accessory_id);
        hk_mem_append(accessory_info, keys->device_session_key_public);
    }

    // // spec 5.7.2.4
    RUN_AND_CHECK(ret, hk_ed25519_init_from_keys, accessory_long_term_key, accessory_private_key, accessory_public_key);
    RUN_AND_CHECK(ret, hk_ed25519_sign, accessory_long_term_key, accessory_info, accessory_signature);

    // spec 5.7.2.5
    if (!ret)
    {
        tlv_data_response_sub = hk_tlv_add_mem(tlv_data_response_sub, HK_PAIR_TLV_IDENTIFIER, accessory_id);
        tlv_data_response_sub = hk_tlv_add_mem(tlv_data_response_sub, HK_PAIR_TLV_SIGNATURE, accessory_signature);

        hk_tlv_serialize(tlv_data_response_sub, sub_result);
    }

    // spec 5.7.2.6
    RUN_AND_CHECK(ret, hk_hkdf, keys->accessory_shared_secret, keys->session_key, HK_HKDF_PAIR_VERIFY_ENCRYPT_SALT, HK_HKDF_PAIR_VERIFY_ENCRYPT_INFO);

    // spec 5.7.2.7
    RUN_AND_CHECK(ret, hk_chacha20poly1305_encrypt, keys->session_key, HK_CHACHA_VERIFY_MSG2, sub_result, encrypted);

    // spec 5.7.2.8
    tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M2);
    if (!ret)
    {
        tlv_data_response = hk_tlv_add_mem(tlv_data_response, HK_PAIR_TLV_PUBLICKEY, keys->accessory_session_key_public); // there is an error in the specification, dont use the srp proof
        tlv_data_response = hk_tlv_add_mem(tlv_data_response, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted);
    }
    else
    {
        tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_ERROR, HK_PAIR_TLV_ERROR_AUTHENTICATION);
    }

    hk_tlv_serialize(tlv_data_response, response);
    RUN_AND_CHECK(ret, hk_pair_verify_create_session_security, keys);
    
    hk_tlv_free(tlv_data_response_sub);
    hk_tlv_free(tlv_data_response);
    hk_mem_free(accessory_info);
    hk_mem_free(accessory_id);
    hk_mem_free(accessory_signature);
    hk_mem_free(accessory_public_key);
    hk_mem_free(accessory_private_key);
    hk_mem_free(sub_result);
    hk_mem_free(encrypted);
    hk_ed25519_free(accessory_long_term_key);
    hk_curve25519_free(accessory_curve_key_pair);
    hk_curve25519_free(device_curve_key_pair);

    return ret;
}

esp_err_t hk_pair_verify_finish(hk_conn_key_store_t *keys, hk_tlv_t *request_tlvs, hk_mem *response)
{
    HK_LOGV("Now running pair verify finish.");
    hk_ed25519_key_t *device_long_term_key = hk_ed25519_init();
    hk_mem *device_long_term_key_public = hk_mem_init();
    hk_mem *device_info = hk_mem_init();
    hk_mem *device_id = hk_mem_init();

    hk_mem *encrypted_data = hk_mem_init();
    hk_mem *decrypted_data = hk_mem_init();
    hk_mem *device_signature = hk_mem_init();
    hk_tlv_t *request_tlvs_decrypted = NULL;
    hk_tlv_t *tlv_data_response = NULL;

    esp_err_t ret = hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);

    RUN_AND_CHECK(ret, hk_chacha20poly1305_decrypt, keys->session_key, HK_CHACHA_VERIFY_MSG3, encrypted_data, decrypted_data);

    if (!ret)
    {
        request_tlvs_decrypted = hk_tlv_deserialize(decrypted_data);
    }

    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, request_tlvs_decrypted, HK_PAIR_TLV_IDENTIFIER, device_id);
    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, request_tlvs_decrypted, HK_PAIR_TLV_SIGNATURE, device_signature);
    RUN_AND_CHECK(ret, hk_pairings_store_ltpk_get, device_id, device_long_term_key_public);
    RUN_AND_CHECK(ret, hk_ed25519_init_from_public_key, device_long_term_key, device_long_term_key_public);

    if (!ret)
    {
        hk_mem_append(device_info, keys->device_session_key_public);
        hk_mem_append(device_info, device_id);
        hk_mem_append(device_info, keys->accessory_session_key_public);
    }

    RUN_AND_CHECK(ret, hk_ed25519_verify, device_long_term_key, device_signature, device_info);

    tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M4);
    if (ret)
    {
        tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_ERROR, HK_PAIR_TLV_ERROR_AUTHENTICATION);
    }
    else
    {
        hk_pair_verify_create_session(keys);
    }

    hk_tlv_serialize(tlv_data_response, response);

    hk_ed25519_free(device_long_term_key);
    hk_tlv_free(request_tlvs_decrypted);
    hk_tlv_free(tlv_data_response);
    hk_mem_free(device_info);
    hk_mem_free(encrypted_data);
    hk_mem_free(decrypted_data);
    hk_mem_free(device_signature);
    hk_mem_free(device_long_term_key_public);
    hk_mem_free(device_id);

    return ret;
}

esp_err_t hk_pair_verify_resume(hk_conn_key_store_t *keys, hk_tlv_t *request_tlvs, hk_mem *response)
{
    HK_LOGD("Now running pair verify resume.");
    hk_mem *session_id = hk_mem_init();
    hk_mem *encryption_key = hk_mem_init();
    hk_mem *encrypted_data = hk_mem_init();
    hk_mem *device_session_key_public = hk_mem_init();
    hk_mem *salt = hk_mem_init();
    hk_tlv_t *tlv_data_response = NULL;
    esp_err_t ret = ESP_OK;

    ret = hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_SESSIONID, session_id);

    hk_pair_verify_session_t *session = NULL;
    hk_ll_foreach(hk_pair_verify_sessions, s)
    {
        if (hk_mem_equal(s->id, session_id))
        {
            session = s;
            hk_ll_break();
        }
    }

    if (session == NULL)
    {
        HK_LOGD("No session for resume found.");
        ret = ESP_ERR_NOT_FOUND;
    }
    else
    {
        RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, request_tlvs, HK_PAIR_TLV_PUBLICKEY, device_session_key_public);
        RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, request_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);

        if (!ret)
        {
            hk_mem_append(salt, device_session_key_public);
            hk_mem_append(salt, session_id);
        }
        RUN_AND_CHECK(ret, hk_hkdf_with_external_salt, session->accessory_shared_secret, encryption_key, salt, HK_HKDF_PAIR_RESUME_REQUEST_INFO);
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
        RUN_AND_CHECK(ret, hk_hkdf_with_external_salt, session->accessory_shared_secret, encryption_key, salt, HK_HKDF_PAIR_RESUME_RESPONSE_INFO);

        if (!ret)
        {
            hk_mem_set(encrypted_data, 0);
        }

        RUN_AND_CHECK(ret, hk_chacha20poly1305_caluclate_auth_tag_without_message, encryption_key, HK_CHACHA_RESUME_MSG2, encrypted_data);
        RUN_AND_CHECK(ret, hk_hkdf_with_external_salt, session->accessory_shared_secret, keys->accessory_shared_secret, salt, HK_HKDF_PAIR_RESUME_SHARED_SECRET_INFO);

        if (!ret)
        {
            hk_mem_set_mem(session->accessory_shared_secret, keys->accessory_shared_secret);
            RUN_AND_CHECK(ret, hk_pair_verify_create_session_security, keys);
        }

        tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M2);
        if (!ret)
        {
            tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_METHOD, HK_PAIR_TLV_METHOD_RESUME);
            tlv_data_response = hk_tlv_add_mem(tlv_data_response, HK_PAIR_TLV_SESSIONID, session->id);
            tlv_data_response = hk_tlv_add_mem(tlv_data_response, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);
            HK_LOGD("Resuming session.");
        }
        else
        {
            if (session != NULL)
            {
                hk_mem_free(session->id);
                hk_mem_free(session->accessory_shared_secret);
                hk_ll_remove(hk_pair_verify_sessions, session);
            }
        }

        hk_tlv_serialize(tlv_data_response, response);
    }

    hk_tlv_free(tlv_data_response);
    hk_mem_free(session_id);
    hk_mem_free(encryption_key);
    hk_mem_free(encrypted_data);
    hk_mem_free(device_session_key_public);
    hk_mem_free(salt);

    return ret;
}

esp_err_t hk_pair_verify(hk_mem *request, hk_mem *result, hk_conn_key_store_t *keys, bool *is_session_encrypted)
{
    esp_err_t ret = ESP_OK;
    hk_tlv_t *tlv_data_request = hk_tlv_deserialize(request);
    hk_tlv_t *state_tlv = hk_tlv_get_tlv_by_type(tlv_data_request, HK_PAIR_TLV_STATE);

    if (state_tlv == NULL)
    {
        HK_LOGE("Could not find tlv with type state.");
        ret = ESP_ERR_INVALID_ARG;
    }
    else
    {
        switch (*state_tlv->value)
        {
        case HK_PAIR_TLV_STATE_M1:
        {
            hk_tlv_t *method_tlv = hk_tlv_get_tlv_by_type(tlv_data_request, HK_PAIR_TLV_METHOD);
            if (method_tlv != NULL && method_tlv->length > 0)
            {
                if (*method_tlv->value == HK_PAIR_TLV_METHOD_RESUME)
                {
                    ret = hk_pair_verify_resume(keys, tlv_data_request, result);
                    if (ret == ESP_OK)
                    {
                        *is_session_encrypted = true;
                    }
                }
            }
            
            if(!*is_session_encrypted)
            {
                ret = hk_pair_verify_start(keys, tlv_data_request, result);
            }

            break;
        }
        case HK_PAIR_TLV_STATE_M3:
        {
            ret = hk_pair_verify_finish(keys, tlv_data_request, result);

            if (ret == ESP_OK)
            {
                *is_session_encrypted = true;
                HK_LOGV("Pair verify succeeded.");
            }
            break;
        }
        default:
            HK_LOGE("Unexpected value in tlv in pair setup: %d", *state_tlv->value);
            ret = ESP_ERR_INVALID_ARG;
        }
    }


    hk_tlv_free(tlv_data_request);

    return ret;
}
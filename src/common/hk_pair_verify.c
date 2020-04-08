#include "hk_pair_verify.h"

#include "../utils/hk_logging.h"
#include "../utils/hk_tlv.h"
#include "../utils/hk_util.h"
#include "../utils/hk_store.h"
#include "../utils/hk_res.h"
#include "../utils/hk_ll.h"
#include "../crypto/hk_curve25519.h"
#include "../crypto/hk_hkdf.h"
#include "../crypto/hk_chacha20poly1305.h"
#include "../crypto/hk_ed25519.h"
#include "hk_pairings_store.h"
#include "hk_pair_tlvs.h"

typedef struct
{
    hk_mem *id;
    hk_mem *shared_secret;
} hk_pair_verify_session_t;

hk_pair_verify_session_t *hk_pair_verify_sessions = NULL;

esp_err_t hk_pair_verify_create_session(hk_pair_verify_keys_t *keys)
{
    esp_err_t ret = ESP_OK;

    hk_mem *session_id = hk_mem_create();
    ret = hk_hkdf_with_given_size(keys->shared_secret, session_id, 8, HK_HKDF_PAIR_VERIFY_RESUME_SALT, HK_HKDF_PAIR_VERIFY_RESUME_INFO);

    if (!ret && hk_pair_verify_sessions != NULL && hk_ll_count(hk_pair_verify_sessions) >= 8)
    {
        HK_LOGE("To many sessions and automatic deletion strategy is still to be implemented.");
    }

    if (!ret)
    {
        hk_pair_verify_sessions = hk_ll_new(hk_pair_verify_sessions);
        hk_pair_verify_sessions->id = hk_mem_create();
        hk_pair_verify_sessions->shared_secret = hk_mem_create();
        hk_mem_append(hk_pair_verify_sessions->id, session_id);
        hk_mem_append(hk_pair_verify_sessions->shared_secret, keys->shared_secret);
        HK_LOGW("Initial session created");
        hk_mem_log("session_id", hk_pair_verify_sessions->id);
        hk_mem_log("shared_secret", hk_pair_verify_sessions->shared_secret);
    }

    hk_mem_free(session_id);

    return ret;
}

esp_err_t hk_create_session_security(hk_pair_verify_keys_t *keys)
{
    esp_err_t ret = ESP_OK;

    ret = hk_hkdf(keys->shared_secret, keys->response_key, HK_HKDF_CONTROL_READ_SALT, HK_HKDF_CONTROL_READ_INFO);

    if (!ret)
    {
        ret = hk_hkdf(keys->shared_secret, keys->request_key, HK_HKDF_CONTROL_WRITE_SALT, HK_HKDF_CONTROL_WRITE_INFO);
    }

    return ret;
}

esp_err_t hk_pair_verify_start(hk_pair_verify_keys_t *keys, hk_tlv_t *request_tlvs, hk_mem *result)
{
    HK_LOGD("Now running pair verify start.");
    esp_err_t ret = ESP_OK;
    hk_ed25519_key_t *accessory_key = hk_ed25519_init_key();
    hk_curve25519_key_t *accessory_curve = hk_curve25519_init_key();
    hk_curve25519_key_t *device_curve = hk_curve25519_init_key();
    hk_mem *accessory_info = hk_mem_create();
    hk_mem *accessory_id = hk_mem_create();
    hk_mem *accessory_signature = hk_mem_create();
    hk_mem *accessory_public_key = hk_mem_create();
    hk_mem *accessory_private_key = hk_mem_create();
    hk_mem *sub_result = hk_mem_create();
    hk_mem *encrypted = hk_mem_create();
    hk_tlv_t *sub_tlv_data = NULL;
    hk_tlv_t *tlv_data = NULL;

    hk_pair_verify_keys_reset(keys);
    keys->session_key = hk_mem_create();
    keys->accessory_curve_public_key = hk_mem_create();
    keys->device_curve_public_key = hk_mem_create();

    if (hk_store_keys_can_get())
    {
        hk_store_key_pub_get(accessory_public_key);
        hk_store_key_priv_get(accessory_private_key);
    }
    else
    {
        HK_LOGE("Cannot get key from store for pairing.");
        ret = HK_RES_UNKNOWN;
    }

    if (accessory_public_key->size < 1 || accessory_private_key->size < 1)
    {
        HK_LOGE("Verfiy was called, but accessory keys are not present.");
        ret = HK_RES_UNKNOWN;
    }

    if (!ret)
    {
        ret = hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_PUBLICKEY, keys->device_curve_public_key);
    }
    hk_mem_log("device_curve_public_key", keys->device_curve_public_key);
    if (!ret)
        ret = hk_curve25519_generate_key_from_public_key(keys->device_curve_public_key, device_curve);

    if (!ret)
        ret = hk_curve25519_generate_key(accessory_curve);

    if (!ret)
        ret = hk_curve25519_shared_secret(accessory_curve, device_curve, keys->shared_secret);

    if (!ret)
        ret = hk_curve25519_export_public_key(accessory_curve, keys->accessory_curve_public_key);

    if (!ret)
    {
        ret = hk_ed25519_generate_key_from_keys(accessory_key, accessory_private_key, accessory_public_key);
    }

    if (!ret)
    {
        hk_util_get_accessory_id_serialized(accessory_id);
        hk_mem_append(accessory_info, keys->accessory_curve_public_key);
        hk_mem_append(accessory_info, accessory_id);
        hk_mem_append(accessory_info, keys->device_curve_public_key);

        ret = hk_ed25519_sign(accessory_key, accessory_info, accessory_signature);
    }

    if (!ret)
    {
        sub_tlv_data = hk_tlv_add(sub_tlv_data, HK_PAIR_TLV_IDENTIFIER, accessory_id);
        sub_tlv_data = hk_tlv_add(sub_tlv_data, HK_PAIR_TLV_SIGNATURE, accessory_signature);

        hk_tlv_serialize(sub_tlv_data, sub_result);
    }

    if (!ret)
    {
        ret = hk_hkdf(keys->shared_secret, keys->session_key, HK_HKDF_PAIR_VERIFY_ENCRYPT_SALT, HK_HKDF_PAIR_VERIFY_ENCRYPT_INFO);
    }

    if (!ret)
    {
        ret = hk_chacha20poly1305_encrypt(keys->session_key, HK_CHACHA_VERIFY_MSG2, sub_result, encrypted);
    }

    if (!ret)
    {
        ret = hk_create_session_security(keys);
    }

    tlv_data = hk_tlv_add_uint8(tlv_data, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M2);
    if (!ret)
    {
        tlv_data = hk_tlv_add(tlv_data, HK_PAIR_TLV_PUBLICKEY, keys->accessory_curve_public_key); // there is an error in the specification, dont use the srp proof
        tlv_data = hk_tlv_add(tlv_data, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted);
    }
    else
    {
        tlv_data = hk_tlv_add_uint8(tlv_data, HK_PAIR_TLV_ERROR, HK_PAIR_TLV_ERROR_AUTHENTICATION);
    }

    hk_tlv_serialize(tlv_data, result);

    hk_tlv_free(tlv_data);
    hk_tlv_free(sub_tlv_data);
    hk_mem_free(accessory_info);
    hk_mem_free(accessory_id);
    hk_mem_free(accessory_signature);
    hk_mem_free(accessory_public_key);
    hk_mem_free(accessory_private_key);
    hk_mem_free(sub_result);
    hk_mem_free(encrypted);
    hk_ed25519_free_key(accessory_key);
    hk_curve25519_free_key(accessory_curve);
    hk_curve25519_free_key(device_curve);

    return ret;
}

esp_err_t hk_pair_verify_finish(hk_pair_verify_keys_t *keys, hk_tlv_t *request_tlvs, hk_mem *result)
{
    HK_LOGD("Now running pair verify finish.");
    hk_ed25519_key_t *device_key = hk_ed25519_init_key();
    hk_mem *encrypted_data = hk_mem_create();
    hk_mem *decrypted_data = hk_mem_create();
    hk_mem *device_signature = hk_mem_create();
    hk_mem *device_public_key = hk_mem_create();
    hk_mem *device_info = hk_mem_create();
    hk_mem *device_id = hk_mem_create();
    hk_tlv_t *tlv_data_decrypted = NULL;
    hk_tlv_t *tlv_data = NULL;

    esp_err_t ret = hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);

    if (!ret)
    {
        HK_LOGV("Decrypting pairing request.");
        hk_chacha20poly1305_decrypt(keys->session_key, HK_CHACHA_VERIFY_MSG3, encrypted_data, decrypted_data);
        tlv_data_decrypted = hk_tlv_deserialize(decrypted_data);
        ret = hk_tlv_get_mem_by_type(tlv_data_decrypted, HK_PAIR_TLV_IDENTIFIER, device_id);
    }

    if (!ret)
    {
        HK_LOGV("Getting device signature.");
        ret = hk_tlv_get_mem_by_type(tlv_data_decrypted, HK_PAIR_TLV_SIGNATURE, device_signature);
    }

    if (!ret)
    {
        HK_LOGV("Getting long term public key.");
        ret = hk_pairings_store_ltpk_get(device_id, device_public_key);
    }

    if (!ret)
    {
        HK_LOGV("Generating key.");
        ret = hk_ed25519_generate_key_from_public_key(device_key, device_public_key);
    }

    if (!ret)
    {
        HK_LOGV("Verifying key.");
        hk_mem_append(device_info, keys->device_curve_public_key);
        hk_mem_append(device_info, device_id);
        hk_mem_append(device_info, keys->accessory_curve_public_key);
        ret = hk_ed25519_verify(device_key, device_info, device_signature);
    }

    tlv_data = hk_tlv_add_uint8(tlv_data, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M4);
    if (ret)
    {
        tlv_data = hk_tlv_add_uint8(tlv_data, HK_PAIR_TLV_ERROR, HK_PAIR_TLV_ERROR_AUTHENTICATION);
    }
    else
    {
        hk_pair_verify_create_session(keys);
    }

    HK_LOGV("Serializing answer.");
    hk_tlv_serialize(tlv_data, result);
    HK_LOGV("Done.");

    hk_ed25519_free_key(device_key);
    hk_tlv_free(tlv_data);
    hk_tlv_free(tlv_data_decrypted);
    hk_mem_free(device_info);
    hk_mem_free(encrypted_data);
    hk_mem_free(decrypted_data);
    hk_mem_free(device_signature);
    hk_mem_free(device_public_key);
    hk_mem_free(device_id);

    hk_mem_free(keys->session_key);
    hk_mem_free(keys->accessory_curve_public_key);
    hk_mem_free(keys->device_curve_public_key);

    return ret;
}

esp_err_t hk_pair_verify_resume(hk_pair_verify_keys_t *keys, hk_tlv_t *request_tlvs, hk_mem *result)
{
    hk_mem *session_id = hk_mem_create();
    hk_mem *encryption_key = hk_mem_create();
    hk_mem *encrypted_data = hk_mem_create();
    hk_mem *decrypted_data = hk_mem_create();
    hk_mem_append_string(decrypted_data, "01234567890123456789012345678901");
    hk_mem *device_curve_public_key = hk_mem_create();
    hk_mem *salt = hk_mem_create();
    hk_tlv_t *response_tlvs = NULL;
    esp_err_t ret = ESP_OK;

    ret = hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_SESSIONID, session_id);

    HK_LOGW("received session_id");
    hk_mem_log("session_id", session_id);
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
    else
    {
        hk_mem_log("Found session for resuming.", session_id);
    }

    if (!ret)
    {
        ret = hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_PUBLICKEY, device_curve_public_key);
        hk_mem_log("device_curve_public_key", device_curve_public_key);
    }

    if (!ret)
    {
        ret = hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);
        hk_mem_log("encryption_data", encrypted_data);
    }

    if (!ret)
    {
        HK_LOGD("Deriving encryption key as device did.");
        hk_mem_append(salt, device_curve_public_key);
        hk_mem_append(salt, session_id);
        hk_hkdf_with_external_salt(session->shared_secret, encryption_key, salt, HK_HKDF_PAIR_RESUME_REQUEST_INFO);
    }

    if (!ret)
    {
        HK_LOGD("Verifying auth tag.");
        ret = hk_chacha20poly1305_verify_auth_tag(encryption_key, HK_CHACHA_RESUME_MSG1, encrypted_data);
        HK_LOGD("Verify result: %d.", ret);
    }

    if (!ret)
    {
        HK_LOGD("Generating new session id.");
        hk_mem_set(session->id, 0);
        uint32_t random_number = esp_random();
        hk_mem_append_buffer(session->id, &random_number, sizeof(uint32_t));
        random_number = esp_random();
        hk_mem_append_buffer(session->id, &random_number, sizeof(uint32_t));
        hk_mem_log("new session id", session->id);
    }

    if (!ret)
    {
        HK_LOGD("Deriving new encryption key with new session id.");
        hk_mem_set(salt, 0);
        hk_mem_set(encryption_key, 0);
        hk_mem_append(salt, device_curve_public_key);
        hk_mem_append(salt, session->id);
        hk_mem_log("salt for response key", salt);
        hk_hkdf_with_external_salt(session->shared_secret, encryption_key, salt, HK_HKDF_PAIR_RESUME_RESPONSE_INFO);
        hk_mem_log("response key", encryption_key);
    }

    if (!ret)
    {
        HK_LOGD("Calculating auth tag without message.");
        hk_mem_set(encrypted_data, 0);
        ret = hk_chacha20poly1305_caluclate_auth_tag_without_message(encryption_key, HK_CHACHA_RESUME_MSG2, encrypted_data);
        hk_mem_log("RESPONSE", encrypted_data);
    }

    if (!ret)
    {
        HK_LOGD("Calculate new shared secret.");
        hk_pair_verify_keys_reset(keys);
        hk_hkdf_with_external_salt(session->shared_secret, keys->shared_secret, salt, HK_HKDF_PAIR_RESUME_SHARED_SECRET_INFO);
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

        HK_LOGD("Serializing answer.");
        hk_tlv_serialize(response_tlvs, result);
        HK_LOGI("Done, sending resume.");

        hk_tlv_log("pair_verify response tlvs", response_tlvs, true, false);
    }
    else
    {
        // todo: send error if session was found, but encryption failed
        if (session != NULL)
        {
            HK_LOGD("Invalidating session.");
            hk_mem_free(session->id);
            hk_mem_free(session->shared_secret);
            hk_ll_remove(hk_pair_verify_sessions, session);
        }
    }

    hk_mem_free(session_id);
    hk_mem_free(encryption_key);
    hk_mem_free(encrypted_data);
    hk_mem_free(decrypted_data);
    hk_mem_free(device_curve_public_key);
    hk_mem_free(salt);
    hk_tlv_free(response_tlvs);

    return ret;
}

int hk_pair_verify(hk_mem *request, hk_pair_verify_keys_t *keys, hk_mem *result, bool *is_session_encrypted)
{
    esp_err_t res = HK_RES_OK;
    hk_tlv_t *request_tlvs = hk_tlv_deserialize(request);
    hk_tlv_t *state_tlv = hk_tlv_get_tlv_by_type(request_tlvs, HK_PAIR_TLV_STATE);
    hk_tlv_log("pair_verify request tlvs", request_tlvs, true, false);
    if (state_tlv == NULL)
    {
        HK_LOGE("Could not find tlv with type state.");
        res = HK_RES_MALFORMED_REQUEST;
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
                    res = hk_pair_verify_resume(keys, request_tlvs, result);
                    if (res == ESP_OK)
                    {
                        *is_session_encrypted = true;
                    }
                    else
                    {
                        res = ESP_OK;
                        res = hk_pair_verify_start(keys, request_tlvs, result);
                    }
                }
            }
            else
            {
                res = hk_pair_verify_start(keys, request_tlvs, result);
            }
            break;
        }
        case HK_PAIR_TLV_STATE_M3:
        {
            res = hk_pair_verify_finish(keys, request_tlvs, result);

            if (res == ESP_OK)
            {
                *is_session_encrypted = true;
            }
            break;
        }
        default:
            HK_LOGE("Unexpected value in tlv in pair setup: %d", *state_tlv->value);
            res = HK_RES_MALFORMED_REQUEST;
        }
    }

    hk_tlv_free(request_tlvs);

    return res;
}
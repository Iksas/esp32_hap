#include "hk_pair_setup.h"

#include <string.h>
#include "../utils/hk_logging.h"
#include "../utils/hk_tlv.h"
#include "../utils/hk_util.h"
#include "../include/hk_mem.h"
#include "../crypto/hk_srp.h"
#include "../crypto/hk_hkdf.h"
#include "../crypto/hk_ed25519.h"
#include "../crypto/hk_chacha20poly1305.h"

#include "hk_accessory_id.h"
#include "hk_pairings_store.h"
#include "hk_pair_tlvs.h"
#include "hk_code_store.h"
#include "hk_key_store.h"

static esp_err_t hk_pairing_setup_srp_start(hk_mem *result, hk_conn_key_store_t *keys)
{
    HK_LOGI("Starting pairing");
    HK_LOGD("pairing setup 1/3 (start).");

    esp_err_t ret = ESP_OK;
    keys->pair_setup_public_key = hk_mem_init();
    hk_mem *salt = hk_mem_init();
    hk_tlv_t *tlv_data_response = NULL;

    // init
    keys->pair_setup_srp_key = hk_srp_init_key();
    RUN_AND_CHECK(ret, hk_srp_generate_key, keys->pair_setup_srp_key, "Pair-Setup", hk_code); // username has to be Pair-Setup according to specification
    RUN_AND_CHECK(ret, hk_srp_export_public_key, keys->pair_setup_srp_key, keys->pair_setup_public_key);
    RUN_AND_CHECK(ret, hk_srp_export_salt, keys->pair_setup_srp_key, salt);

    tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M2);
    if (!ret)
    {
        tlv_data_response = hk_tlv_add(tlv_data_response, HK_PAIR_TLV_PUBLICKEY, keys->pair_setup_public_key);
        tlv_data_response = hk_tlv_add(tlv_data_response, HK_PAIR_TLV_SALT, salt);
    }
    else
    {
        tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_ERROR, HK_PAIR_TLV_ERROR_UNKNOWN);
    }

    hk_tlv_serialize(tlv_data_response, result);

    hk_tlv_free(tlv_data_response);
    hk_mem_free(salt);

    HK_LOGD("pairing setup 1/3 done.");

    return ret;
}

static esp_err_t hk_pairing_setup_srp_verify(hk_tlv_t *tlv, hk_mem *result, hk_conn_key_store_t *keys)
{
    HK_LOGD("pairing setup 2/3 (verifying).");

    esp_err_t ret = ESP_OK;
    hk_mem *ios_pk = hk_mem_init();
    hk_mem *ios_proof = hk_mem_init();
    hk_mem *accessory_proof = hk_mem_init();
    hk_tlv_t *tlv_data_response = NULL;
    bool valid = true;

    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, tlv, HK_PAIR_TLV_PUBLICKEY, ios_pk);
    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, tlv, HK_PAIR_TLV_PROOF, ios_proof);
    RUN_AND_CHECK(ret, hk_srp_compute_key, keys->pair_setup_srp_key, keys->pair_setup_public_key, ios_pk);
    RUN_AND_CHECK(ret, hk_srp_verify, keys->pair_setup_srp_key, ios_proof, &valid);
    RUN_AND_CHECK(ret, hk_srp_export_proof, keys->pair_setup_srp_key, accessory_proof);

    tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M4);
    if (!ret)
    {
        tlv_data_response = hk_tlv_add(tlv_data_response, HK_PAIR_TLV_PROOF, accessory_proof);
    }
    else
    {
        tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_ERROR, valid ? HK_PAIR_TLV_ERROR_UNKNOWN : HK_PAIR_TLV_ERROR_AUTHENTICATION);
    }

    hk_tlv_serialize(tlv_data_response, result);

    hk_tlv_free(tlv_data_response);
    hk_mem_free(keys->pair_setup_public_key);
    hk_mem_free(ios_pk);
    hk_mem_free(ios_proof);
    hk_mem_free(accessory_proof);

    HK_LOGD("pairing setup 2/3 done.");

    return ret;
}

static esp_err_t hk_pairing_setup_exchange_response_verification(hk_tlv_t *tlv, hk_mem *shared_secret, hk_mem *srp_private_key, hk_mem *device_id)
{
    esp_err_t ret = ESP_OK;
    hk_mem *encrypted_data = hk_mem_init();
    hk_mem *decrypted_data = hk_mem_init();
    hk_mem *device_long_term_key_public = hk_mem_init();
    hk_mem *device_signature = hk_mem_init();
    hk_mem *device_info = hk_mem_init();
    hk_mem *device_x = hk_mem_init();
    hk_tlv_t *tlv_data_decrypted = NULL;
    hk_ed25519_key_t *device_key = hk_ed25519_init();

    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, tlv, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);
    RUN_AND_CHECK(ret, hk_hkdf, srp_private_key, shared_secret, HK_HKDF_PAIR_SETUP_ENCRYPT_SALT, HK_HKDF_PAIR_SETUP_ENCRYPT_INFO);
    RUN_AND_CHECK(ret, hk_chacha20poly1305_decrypt, shared_secret, HK_CHACHA_SETUP_MSG5, encrypted_data, decrypted_data);
    
    if (!ret)
    {
        tlv_data_decrypted = hk_tlv_deserialize(decrypted_data);
    }

    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, tlv_data_decrypted, HK_PAIR_TLV_IDENTIFIER, device_id);
    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, tlv_data_decrypted, HK_PAIR_TLV_PUBLICKEY, device_long_term_key_public);
    RUN_AND_CHECK(ret, hk_tlv_get_mem_by_type, tlv_data_decrypted, HK_PAIR_TLV_SIGNATURE, device_signature);
    RUN_AND_CHECK(ret, hk_ed25519_init_from_public_key, device_key, device_long_term_key_public);
    RUN_AND_CHECK(ret, hk_hkdf, srp_private_key, device_x, HK_HKDF_PAIR_SETUP_CONTROLLER_SALT, HK_HKDF_PAIR_SETUP_CONTROLLER_INFO);

    if (!ret)
    {
        hk_mem_append(device_info, device_x);
        hk_mem_append(device_info, device_id);
        hk_mem_append(device_info, device_long_term_key_public);
    }

    RUN_AND_CHECK(ret, hk_ed25519_verify, device_key, device_signature, device_info);

    if (!ret)
    {
        hk_pairings_store_add(device_id, device_long_term_key_public, true);
    }

    hk_tlv_free(tlv_data_decrypted);
    hk_mem_free(encrypted_data);
    hk_mem_free(decrypted_data);
    hk_mem_free(device_long_term_key_public);
    hk_mem_free(device_signature);
    hk_mem_free(device_info);
    hk_mem_free(device_x);
    hk_ed25519_free(device_key);

    return ret;
}

static esp_err_t hk_pairing_setup_exchange_response_generation(hk_mem *result, hk_mem *shared_secret, hk_mem *srp_private_key)
{
    esp_err_t ret = ESP_OK;
    hk_ed25519_key_t *accessory_key = hk_ed25519_init();
    hk_mem *accessory_public_key = hk_mem_init();
    hk_mem *accessory_private_key = hk_mem_init();
    hk_mem *accessory_info = hk_mem_init();
    hk_mem *accessory_signature = hk_mem_init();
    hk_mem *accessory_id = hk_mem_init();
    hk_mem *sub_result = hk_mem_init();
    hk_mem *encrypted = hk_mem_init();
    hk_tlv_t *tlv_data_response = NULL;
    hk_tlv_t *tlv_data_response_sub = NULL;

    RUN_AND_CHECK(ret, hk_ed25519_init_from_random, accessory_key);
    RUN_AND_CHECK(ret, hk_ed25519_export_public_key, accessory_key, accessory_public_key);
    RUN_AND_CHECK(ret, hk_key_store_pub_set, accessory_public_key);
    RUN_AND_CHECK(ret, hk_ed25519_export_private_key, accessory_key, accessory_private_key);
    RUN_AND_CHECK(ret, hk_key_store_priv_set, accessory_private_key);
    RUN_AND_CHECK(ret, hk_hkdf, srp_private_key, accessory_info, HK_HKDF_PAIR_SETUP_ACCESSORY_SALT, HK_HKDF_PAIR_SETUP_ACCESSORY_INFO);
    RUN_AND_CHECK(ret, hk_accessory_id_get_serialized, accessory_id);

    if (!ret)
    {
        hk_mem_append_buffer(accessory_info, accessory_id->ptr, accessory_id->size);
        hk_mem_append_buffer(accessory_info, accessory_public_key->ptr, accessory_public_key->size);
    }

    RUN_AND_CHECK(ret, hk_ed25519_sign, accessory_key, accessory_info, accessory_signature);

    if (!ret)
    {
        tlv_data_response_sub = hk_tlv_add(tlv_data_response_sub, HK_PAIR_TLV_IDENTIFIER, accessory_id);
        tlv_data_response_sub = hk_tlv_add(tlv_data_response_sub, HK_PAIR_TLV_PUBLICKEY, accessory_public_key);
        tlv_data_response_sub = hk_tlv_add(tlv_data_response_sub, HK_PAIR_TLV_SIGNATURE, accessory_signature);

        hk_tlv_serialize(tlv_data_response_sub, sub_result);
    }

    RUN_AND_CHECK(ret, hk_chacha20poly1305_encrypt, shared_secret, HK_CHACHA_SETUP_MSG6, sub_result, encrypted);

    tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M6);
    if (!ret)
    {
        tlv_data_response = hk_tlv_add(tlv_data_response, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted);
    }
    else
    {
        tlv_data_response = hk_tlv_add_uint8(tlv_data_response, HK_PAIR_TLV_ERROR, HK_PAIR_TLV_ERROR_AUTHENTICATION);
    }

    hk_tlv_serialize(tlv_data_response, result);

    hk_tlv_free(tlv_data_response_sub);
    hk_tlv_free(tlv_data_response);
    hk_mem_free(accessory_id);
    hk_mem_free(accessory_public_key);
    hk_mem_free(accessory_private_key);
    hk_mem_free(accessory_info);
    hk_mem_free(accessory_signature);
    hk_mem_free(sub_result);
    hk_mem_free(encrypted);
    hk_ed25519_free(accessory_key);

    return ret;
}

static esp_err_t hk_pairing_setup_exchange_response(hk_tlv_t *tlv, hk_mem *result, hk_conn_key_store_t *keys, hk_mem *device_id, bool *is_paired)
{
    HK_LOGD("pairing setup 3/3 (exchange response).");

    hk_mem *shared_secret = hk_mem_init();
    hk_mem *srp_private_key = hk_mem_init();

    esp_err_t ret = ESP_OK;

    RUN_AND_CHECK(ret, hk_srp_export_private_key, keys->pair_setup_srp_key, srp_private_key);
    RUN_AND_CHECK(ret, hk_pairing_setup_exchange_response_verification, tlv, shared_secret, srp_private_key, device_id);
    RUN_AND_CHECK(ret, hk_pairing_setup_exchange_response_generation, result, shared_secret, srp_private_key);

    if (!ret)
    {
        *is_paired = true;
        HK_LOGI("Accessory paired.");
    }

    hk_mem_free(shared_secret);
    hk_mem_free(srp_private_key);
    hk_srp_free_key(keys->pair_setup_srp_key);

    HK_LOGD("pairing setup 3/3 done.");
    return ret;
}

esp_err_t hk_pair_setup(hk_mem *request, hk_mem *response, hk_conn_key_store_t *keys, hk_mem *device_id, bool *is_paired)
{
    hk_tlv_t *tlv_data_request = hk_tlv_deserialize(request);
    hk_tlv_t *type_tlv = hk_tlv_get_tlv_by_type(tlv_data_request, HK_PAIR_TLV_STATE);
    esp_err_t ret = ESP_OK;

    if (type_tlv == NULL)
    {
        HK_LOGE("Could not find tlv with type state.");
        ret = ESP_ERR_INVALID_ARG;
    }
    else
    {
        switch (*type_tlv->value)
        {
        case HK_PAIR_TLV_STATE_M1:
            RUN_AND_CHECK(ret, hk_pairing_setup_srp_start, response, keys);
            break;
        case HK_PAIR_TLV_STATE_M3:
            RUN_AND_CHECK(ret, hk_pairing_setup_srp_verify, tlv_data_request, response, keys);
            break;
        case HK_PAIR_TLV_STATE_M5:
            RUN_AND_CHECK(ret, hk_pairing_setup_exchange_response, tlv_data_request, response, keys, device_id, is_paired);
            break;
        default:
            HK_LOGE("Unexpected value in tlv in pair setup: %d", *type_tlv->value);
            ret = ESP_ERR_INVALID_ARG;
        }
    }

    hk_tlv_free(tlv_data_request);

    return ret;
}
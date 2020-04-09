#include "unity.h"

#include "../../src/common/hk_pair_verify_keys.h"
#include "../../src/common/hk_pair_tlvs.h"
#include "../../src/common/hk_pairings_store.h"
#include "../../src/crypto/hk_ed25519.h"
#include "../../src/crypto/hk_curve25519.h"
#include "../../src/crypto/hk_hkdf.h"
#include "../../src/crypto/hk_chacha20poly1305.h"
#include "../../src/utils/hk_tlv.h"
#include "../../src/utils/hk_store.h"
#include "../../src/utils/hk_logging.h"

#include "hk_pair_verify_test_vectors.h"

esp_err_t hk_pair_verify_start(hk_pair_verify_keys_t *keys, hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr);
esp_err_t hk_pair_verify_finish(hk_pair_verify_keys_t *keys, hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr);
void hk_pair_verify_device_m1(hk_tlv_t **response_tlvs_ptr, hk_curve25519_key_t *device_session_key);
void hk_pair_verify_device_m3(hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr,
                              hk_curve25519_key_t *device_session_key, hk_ed25519_key_t *device_long_term_key);

TEST_CASE("verify once", "[pair] [verify]")
{
    // prepare
    hk_tlv_t *m1_tlvs = NULL;
    hk_tlv_t *m2_tlvs = NULL;
    hk_tlv_t *m3_tlvs = NULL;
    hk_tlv_t *m4_tlvs = NULL;

    hk_pair_verify_keys_t *keys = hk_pair_verify_keys_init();

    hk_curve25519_key_t *device_session_key = hk_curve25519_init();
    hk_ed25519_key_t *device_long_term_key = hk_ed25519_init();
    hk_ed25519_generate_key(device_long_term_key);
    hk_mem *device_long_term_key_public = hk_mem_init();
    hk_ed25519_export_public_key(device_long_term_key, device_long_term_key_public);

    hk_mem *device_id = hk_mem_init();
    hk_mem_append_string(device_id, device_id_string);

    hk_mem *accessory_public_key = hk_mem_init();
    hk_mem_append_buffer(accessory_public_key, (void *)hk_pair_verify_test_accessory_ltpk_bytes, sizeof(hk_pair_verify_test_accessory_ltpk_bytes));
    hk_mem *accessory_private_key = hk_mem_init();
    hk_mem_append_buffer(accessory_private_key, (void *)accessory_private_key_bytes, sizeof(accessory_private_key_bytes));

    hk_store_init();
    hk_pairings_store_remove_all();
    hk_store_key_pub_set(accessory_public_key);
    hk_store_key_priv_set(accessory_private_key);
    hk_pairings_store_add(device_id, device_long_term_key_public, true);

    // test
    hk_pair_verify_device_m1(&m1_tlvs, device_session_key);
    esp_err_t ret1 = hk_pair_verify_start(keys, m1_tlvs, &m2_tlvs);
    hk_pair_verify_device_m3(m2_tlvs, &m3_tlvs, device_session_key, device_long_term_key);
    esp_err_t ret2 = hk_pair_verify_finish(keys, m3_tlvs, &m4_tlvs);

    // assert
    TEST_ASSERT_EQUAL(ret1, ESP_OK);
    TEST_ASSERT_EQUAL(ret2, ESP_OK);

    // cleanup
    hk_tlv_free(m1_tlvs);
    hk_tlv_free(m2_tlvs);
    hk_tlv_free(m3_tlvs);
    hk_tlv_free(m4_tlvs);
    hk_pair_verify_keys_free(keys);
    hk_store_free();
    hk_mem_free(accessory_public_key);
    hk_mem_free(accessory_private_key);

    hk_curve25519_free(device_session_key);
    hk_ed25519_free(device_long_term_key);
    hk_mem_free(device_id);
    hk_mem_free(device_long_term_key_public);
}

void hk_pair_verify_device_m1(hk_tlv_t **response_tlvs_ptr, hk_curve25519_key_t *device_session_key)
{
    hk_tlv_t *response_tlvs = NULL;
    hk_mem *device_session_key_public = hk_mem_init();

    hk_curve25519_generate_key(device_session_key);
    hk_curve25519_export_public_key(device_session_key, device_session_key_public);

    response_tlvs = hk_tlv_add_uint8(response_tlvs, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M2);
    response_tlvs = hk_tlv_add(response_tlvs, HK_PAIR_TLV_PUBLICKEY, device_session_key_public);

    *response_tlvs_ptr = response_tlvs;

    hk_mem_free(device_session_key_public);
}

void hk_pair_verify_device_m3(hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr,
                              hk_curve25519_key_t *device_session_key, hk_ed25519_key_t *device_long_term_key)
{
    hk_tlv_t *response_tlvs = NULL;

    hk_curve25519_key_t *accessory_session_key = hk_curve25519_init();
    hk_ed25519_key_t *accessory_long_term_key = hk_ed25519_init();
    hk_mem *accessory_session_key_public = hk_mem_init();
    // hk_mem *accessory_long_term_public_key = hk_mem_init();
    // hk_mem_append_buffer(accessory_long_term_public_key, (void *)hk_pair_verify_test_accessory_ltpk_bytes,
    //                      sizeof(hk_pair_verify_test_accessory_ltpk_bytes));

    hk_mem *device_session_key_public = hk_mem_init();
    hk_curve25519_export_public_key(device_session_key, device_session_key_public);
    hk_mem *device_id = hk_mem_init();
    hk_mem_append_string(device_id, device_id_string);

    hk_mem *shared_secret = hk_mem_init();
    hk_mem *encrypted_data = hk_mem_init();
    hk_mem *decrypted_data = hk_mem_init();
    hk_mem *session_encryption_key = hk_mem_init();
    hk_mem *device_info = hk_mem_init();
    hk_mem *device_signature = hk_mem_init();
    hk_mem *sub_result = hk_mem_init();

    hk_tlv_t *request_tlvs_decrypted = NULL;
    hk_tlv_t *sub_tlvs = NULL;

    hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_PUBLICKEY, accessory_session_key_public);
    hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);
    hk_curve25519_generate_key_from_public_key(accessory_session_key_public, accessory_session_key);
    hk_curve25519_shared_secret(device_session_key, accessory_session_key, shared_secret);
    hk_hkdf(shared_secret, session_encryption_key, HK_HKDF_PAIR_VERIFY_ENCRYPT_SALT, HK_HKDF_PAIR_VERIFY_ENCRYPT_INFO);
    // hk_chacha20poly1305_decrypt(session_encryption_key, HK_CHACHA_VERIFY_MSG2, encrypted_data, decrypted_data);
    // request_tlvs_decrypted = hk_tlv_deserialize(decrypted_data);
    // hk_tlv_get_mem_by_type(request_tlvs_decrypted, HK_PAIR_TLV_IDENTIFIER, device_id);
    //hk_ed25519_generate_key_from_public_key(accessory_long_term_key, accessory_long_term_public_key);
    hk_mem_append(device_info, device_session_key_public);
    hk_mem_append(device_info, device_id);
    hk_mem_append(device_info, accessory_session_key_public);

    hk_ed25519_sign(device_long_term_key, device_info, device_signature);

    sub_tlvs = hk_tlv_add(sub_tlvs, HK_PAIR_TLV_IDENTIFIER, device_id);
    sub_tlvs = hk_tlv_add(sub_tlvs, HK_PAIR_TLV_SIGNATURE, device_signature);
    hk_tlv_serialize(sub_tlvs, sub_result);
    hk_mem_set(encrypted_data, 0);
    hk_chacha20poly1305_encrypt(session_encryption_key, HK_CHACHA_VERIFY_MSG3, sub_result, encrypted_data);

    response_tlvs = hk_tlv_add_uint8(response_tlvs, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M3);
    response_tlvs = hk_tlv_add(response_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);
    *response_tlvs_ptr = response_tlvs;

    hk_curve25519_free(accessory_session_key);
    hk_ed25519_free(accessory_long_term_key);
    //hk_mem_free(accessory_long_term_public_key);
    hk_mem_free(accessory_session_key_public);

    hk_mem_free(device_id);
    hk_mem_free(device_session_key_public);

    hk_mem_free(shared_secret);
    hk_mem_free(session_encryption_key);
    hk_mem_free(encrypted_data);
    hk_mem_free(decrypted_data);
    hk_mem_free(device_info);
    hk_mem_free(device_signature);
    hk_mem_free(sub_result);

    hk_tlv_free(request_tlvs_decrypted);
    hk_tlv_free(sub_tlvs);
}
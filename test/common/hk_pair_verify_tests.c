// #include "unity.h"

// #include "../../src/common/hk_conn_key_store.h"
// #include "../../src/common/hk_pair_tlvs.h"
// #include "../../src/common/hk_pairings_store.h"
// #include "../../src/crypto/hk_ed25519.h"
// #include "../../src/crypto/hk_curve25519.h"
// #include "../../src/crypto/hk_hkdf.h"
// #include "../../src/crypto/hk_chacha20poly1305.h"
// #include "../../src/utils/hk_tlv.h"
// #include "../../src/utils/hk_store.h"
// #include "../../src/utils/hk_logging.h"

// #include "hk_pair_verify_test_vectors.h"


// esp_err_t hk_pair_verify_start(hk_conn_key_store_t *keys, hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr);
// esp_err_t hk_pair_verify_finish(hk_conn_key_store_t *keys, hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr);
// void hk_pair_verify_device_m1(hk_tlv_t **response_tlvs_ptr, hk_curve25519_key_t *device_session_key);
// void hk_pair_verify_device_m3(hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr,
//                               hk_curve25519_key_t *device_session_key, hk_ed25519_key_t *device_long_term_key);

// TEST_CASE("verify once", "[pair] [verify]")
// {
//     // prepare
//     hk_tlv_t *m1_tlvs = NULL;
//     hk_tlv_t *m2_tlvs = NULL;
//     hk_tlv_t *m3_tlvs = NULL;
//     hk_tlv_t *m4_tlvs = NULL;

//     hk_tlv_t *m12_tlvs = NULL;
//     hk_tlv_t *m22_tlvs = NULL;
//     hk_tlv_t *m32_tlvs = NULL;
//     hk_tlv_t *m42_tlvs = NULL;

//     hk_conn_key_store_t *keys = hk_conn_key_store_init();

//     hk_curve25519_key_t *device_session_key = hk_curve25519_init();
//     hk_ed25519_key_t *device_long_term_key = hk_ed25519_init();
//     hk_ed25519_init_from_random(device_long_term_key);
//     hk_mem *device_long_term_key_public = hk_mem_init();
//     hk_ed25519_export_public_key(device_long_term_key, device_long_term_key_public);

//     hk_mem *device_id = hk_mem_init();
//     hk_mem_append_string(device_id, device_id_string);

//     hk_mem *accessory_public_key = hk_mem_init();
//     hk_mem_append_buffer(accessory_public_key, (void *)hk_pair_verify_test_accessory_ltpk_bytes, sizeof(hk_pair_verify_test_accessory_ltpk_bytes));
//     hk_mem *accessory_private_key = hk_mem_init();
//     hk_mem_append_buffer(accessory_private_key, (void *)accessory_private_key_bytes, sizeof(accessory_private_key_bytes));

//     hk_store_init();
//     hk_pairings_store_remove_all();
//     hk_key_store_pub_set(accessory_public_key);
//     hk_key_store_priv_set(accessory_private_key);
//     hk_pairings_store_add(device_id, device_long_term_key_public, true);

//     // test
//     hk_pair_verify_device_m1(&m1_tlvs, device_session_key);
//     esp_err_t ret1 = hk_pair_verify_start(keys, m1_tlvs, &m2_tlvs);
//     hk_pair_verify_device_m3(m2_tlvs, &m3_tlvs, device_session_key, device_long_term_key);
//     esp_err_t ret2 = hk_pair_verify_finish(keys, m3_tlvs, &m4_tlvs);

//     hk_pair_verify_device_m1(&m12_tlvs, device_session_key);
//     esp_err_t ret12 = hk_pair_verify_start(keys, m12_tlvs, &m22_tlvs);
//     hk_pair_verify_device_m3(m22_tlvs, &m32_tlvs, device_session_key, device_long_term_key);
//     esp_err_t ret22 = hk_pair_verify_finish(keys, m32_tlvs, &m42_tlvs);

//     // assert
//     TEST_ASSERT_EQUAL(ESP_OK, ret1);
//     TEST_ASSERT_EQUAL(ESP_OK, ret2);
//     TEST_ASSERT_EQUAL(ESP_OK, ret12);
//     TEST_ASSERT_EQUAL(ESP_OK, ret22);

//     // cleanup
//     hk_tlv_free(m1_tlvs);
//     hk_tlv_free(m2_tlvs);
//     hk_tlv_free(m3_tlvs);
//     hk_tlv_free(m4_tlvs);
//     hk_conn_key_store_free(keys);
//     hk_store_free();
//     hk_mem_free(accessory_public_key);
//     hk_mem_free(accessory_private_key);

//     hk_curve25519_free(device_session_key);
//     hk_ed25519_free(device_long_term_key);
//     hk_mem_free(device_id);
//     hk_mem_free(device_long_term_key_public);
// }

// void hk_pair_verify_device_m1(hk_tlv_t **response_tlvs_ptr, hk_curve25519_key_t *device_session_key)
// {
//     hk_tlv_t *response_tlvs = NULL;
//     hk_mem *device_session_key_public = hk_mem_init();

//     TEST_ASSERT_EQUAL(ESP_OK, hk_curve25519_update_from_random(device_session_key));
//     TEST_ASSERT_EQUAL(ESP_OK, hk_curve25519_export_public_key(device_session_key, device_session_key_public));

//     response_tlvs = hk_tlv_add_uint8(response_tlvs, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M2);
//     response_tlvs = hk_tlv_add_mem(response_tlvs, HK_PAIR_TLV_PUBLICKEY, device_session_key_public);

//     *response_tlvs_ptr = response_tlvs;

//     hk_mem_free(device_session_key_public);
// }

// void hk_pair_verify_device_m3(hk_tlv_t *request_tlvs, hk_tlv_t **response_tlvs_ptr,
//                               hk_curve25519_key_t *device_session_key, hk_ed25519_key_t *device_long_term_key)
// {
//     hk_tlv_t *response_tlvs = NULL;

//     hk_curve25519_key_t *accessory_session_key = hk_curve25519_init();
//     hk_ed25519_key_t *accessory_long_term_key = hk_ed25519_init();
//     hk_mem *accessory_session_key_public = hk_mem_init();
//     hk_mem *accessory_id = hk_mem_init();
//     hk_mem *accessory_long_term_public_key = hk_mem_init();
//     hk_mem_append_buffer(accessory_long_term_public_key, (void *)hk_pair_verify_test_accessory_ltpk_bytes,
//                          sizeof(hk_pair_verify_test_accessory_ltpk_bytes));

//     hk_mem *device_session_key_public = hk_mem_init();
//     hk_curve25519_export_public_key(device_session_key, device_session_key_public);
//     hk_mem *device_id = hk_mem_init();
//     hk_mem_append_string(device_id, device_id_string);

//     hk_mem *shared_secret = hk_mem_init();
//     hk_mem *encrypted_data = hk_mem_init();
//     hk_mem *decrypted_data = hk_mem_init();
//     hk_mem *session_encryption_key = hk_mem_init();
//     hk_mem *device_info = hk_mem_init();
//     hk_mem *device_signature = hk_mem_init();
//     hk_mem *sub_result = hk_mem_init();

//     hk_tlv_t *request_sub_tlvs = NULL;
//     hk_tlv_t *response_sub_tlvs = NULL;

//     TEST_ASSERT_EQUAL(ESP_OK, hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_PUBLICKEY, accessory_session_key_public));
//     TEST_ASSERT_EQUAL(ESP_OK, hk_tlv_get_mem_by_type(request_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data));
//     TEST_ASSERT_EQUAL(ESP_OK, hk_curve25519_update_from_public_key(accessory_session_key_public, accessory_session_key));
//     TEST_ASSERT_EQUAL(ESP_OK, hk_curve25519_calculate_shared_secret(device_session_key, accessory_session_key, shared_secret));
//     TEST_ASSERT_EQUAL(ESP_OK, hk_hkdf(shared_secret, session_encryption_key, HK_HKDF_PAIR_VERIFY_ENCRYPT_SALT, HK_HKDF_PAIR_VERIFY_ENCRYPT_INFO));
//     TEST_ASSERT_EQUAL(ESP_OK, hk_chacha20poly1305_decrypt(session_encryption_key, HK_CHACHA_VERIFY_MSG2, encrypted_data, decrypted_data));
//     request_sub_tlvs = hk_tlv_deserialize(decrypted_data);
//     TEST_ASSERT_EQUAL(ESP_OK, hk_tlv_get_mem_by_type(request_sub_tlvs, HK_PAIR_TLV_IDENTIFIER, accessory_id));
//     TEST_ASSERT_EQUAL(ESP_OK, hk_ed25519_init_from_public_key(accessory_long_term_key, accessory_long_term_public_key));
//     hk_mem_append(device_info, device_session_key_public);
//     hk_mem_append(device_info, device_id);
//     hk_mem_append(device_info, accessory_session_key_public);

//     TEST_ASSERT_EQUAL(ESP_OK, hk_ed25519_sign(device_long_term_key, device_info, device_signature));

//     response_sub_tlvs = hk_tlv_add_mem(response_sub_tlvs, HK_PAIR_TLV_IDENTIFIER, device_id);
//     response_sub_tlvs = hk_tlv_add_mem(response_sub_tlvs, HK_PAIR_TLV_SIGNATURE, device_signature);
//     hk_tlv_serialize(response_sub_tlvs, sub_result);
//     hk_mem_set(encrypted_data, 0);
//     TEST_ASSERT_EQUAL(ESP_OK, hk_chacha20poly1305_encrypt(session_encryption_key, HK_CHACHA_VERIFY_MSG3, sub_result, encrypted_data));

//     response_tlvs = hk_tlv_add_uint8(response_tlvs, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M3);
//     response_tlvs = hk_tlv_add_mem(response_tlvs, HK_PAIR_TLV_ENCRYPTEDDATA, encrypted_data);
//     *response_tlvs_ptr = response_tlvs;

//     hk_curve25519_free(accessory_session_key);
//     hk_ed25519_free(accessory_long_term_key);
//     hk_mem_free(accessory_long_term_public_key);
//     hk_mem_free(accessory_session_key_public);
//     hk_mem_free(accessory_id);

//     hk_mem_free(device_id);
//     hk_mem_free(device_session_key_public);

//     hk_mem_free(shared_secret);
//     hk_mem_free(session_encryption_key);
//     hk_mem_free(encrypted_data);
//     hk_mem_free(decrypted_data);
//     hk_mem_free(device_info);
//     hk_mem_free(device_signature);
//     hk_mem_free(sub_result);

//     hk_tlv_free(request_sub_tlvs);
//     hk_tlv_free(response_sub_tlvs);
// }
#include "unity.h"

#include "../../src/crypto/hk_chacha20poly1305.h"
#include "../../src/include/hk_mem.h"
#include "../../src/utils/hk_logging.h"

const char key_bytes[] = {
    0xf7, 0x7f, 0xf3, 0xd0, 0x2a, 0x58, 0x9a, 0x27,
    0x09, 0x36, 0x0f, 0xf4, 0x4f, 0xd4, 0x24, 0x47,
    0x47, 0xa9, 0x33, 0xab, 0x90, 0x72, 0x72, 0x54,
    0xdc, 0x9a, 0x64, 0xd7, 0x78, 0xc2, 0x1a, 0x04};

const char auth_tag_bytes[] = {
    0x1c, 0xd0, 0x66, 0x0b, 0x7a, 0xab, 0x6f, 0xe3,
    0x59, 0x3e, 0x83, 0x89, 0x26, 0xf9, 0xb9, 0x9c};

TEST_CASE("encrypt->decrypt", "[crypto] [chacha]")
{
    // prepare
    hk_mem *message = hk_mem_create();
    hk_mem_append_string(message, "my message");
    hk_mem *key = hk_mem_create();
    hk_mem_append_string(key, "2adc628560e0b4440640c242a5808bcb144db13fa904b07597e73e482d5cb12e");
    hk_mem *encrypted = hk_mem_create();
    hk_mem *decrypted = hk_mem_create();

    // run
    TEST_ASSERT_EQUAL(hk_chacha20poly1305_encrypt(key, HK_CHACHA_VERIFY_MSG2, message, encrypted), ESP_OK);
    TEST_ASSERT_EQUAL(hk_chacha20poly1305_decrypt(key, HK_CHACHA_VERIFY_MSG2, encrypted, decrypted), ESP_OK);

    // assert
    TEST_ASSERT_TRUE(hk_mem_cmp(decrypted, message));

    // clean
    hk_mem_free(key);
    hk_mem_free(message);
    hk_mem_free(encrypted);
    hk_mem_free(decrypted);
}

TEST_CASE("encrypt->verify", "[crypto] [chacha]")
{
    // prepare
    hk_mem *key = hk_mem_create();
    hk_mem_append_buffer(key, (void *)key_bytes, 32);
    hk_mem *auth_tag = hk_mem_create();
    hk_mem_append_buffer(auth_tag, (void *)auth_tag_bytes, 16);

    // run
    TEST_ASSERT_EQUAL(hk_chacha20poly1305_verify_auth_tag(key, HK_CHACHA_RESUME_MSG1, auth_tag), ESP_OK);

    // clean
    hk_mem_free(key);
    hk_mem_free(auth_tag);
}

TEST_CASE("calculate auth tag", "[crypto] [chacha]")
{
    // prepare
    hk_mem *key = hk_mem_create();
    hk_mem_append_buffer(key, (void *)key_bytes, 32);
    hk_mem *auth_tag = hk_mem_create();
    hk_mem *auth_tag_expected = hk_mem_create();
    hk_mem_append_buffer(auth_tag_expected, (void *)auth_tag_bytes, 16);

    // run
    TEST_ASSERT_EQUAL(hk_chacha20poly1305_caluclate_auth_tag_without_message(key, HK_CHACHA_RESUME_MSG1, auth_tag), ESP_OK);

    // assert
    TEST_ASSERT_TRUE(hk_mem_cmp(auth_tag, auth_tag_expected));

    // clean
    hk_mem_free(key);
    hk_mem_free(auth_tag);
}
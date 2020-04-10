#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"

#define HK_HKDF_PAIR_SETUP_ENCRYPT_SALT "Pair-Setup-Encrypt-Salt"
#define HK_HKDF_PAIR_SETUP_ENCRYPT_INFO "Pair-Setup-Encrypt-Info"
#define HK_HKDF_PAIR_SETUP_CONTROLLER_SALT "Pair-Setup-Controller-Sign-Salt"
#define HK_HKDF_PAIR_SETUP_CONTROLLER_INFO "Pair-Setup-Controller-Sign-Info"
#define HK_HKDF_PAIR_SETUP_ACCESSORY_SALT "Pair-Setup-Accessory-Sign-Salt"
#define HK_HKDF_PAIR_SETUP_ACCESSORY_INFO "Pair-Setup-Accessory-Sign-Info"
#define HK_HKDF_PAIR_VERIFY_ENCRYPT_SALT "Pair-Verify-Encrypt-Salt"
#define HK_HKDF_PAIR_VERIFY_ENCRYPT_INFO "Pair-Verify-Encrypt-Info"
#define HK_HKDF_PAIR_VERIFY_RESUME_SALT "Pair-Verify-ResumeSessionID-Salt"
#define HK_HKDF_PAIR_VERIFY_RESUME_INFO "Pair-Verify-ResumeSessionID-Info"
#define HK_HKDF_PAIR_RESUME_REQUEST_INFO "Pair-Resume-Request-Info"
#define HK_HKDF_PAIR_RESUME_RESPONSE_INFO "Pair-Resume-Response-Info"
#define HK_HKDF_PAIR_RESUME_SHARED_SECRET_SALT "external"
#define HK_HKDF_PAIR_RESUME_SHARED_SECRET_INFO "Pair-Resume-Shared-Secret-Info"
#define HK_HKDF_CONTROL_READ_SALT "Control-Salt"
#define HK_HKDF_CONTROL_READ_INFO "Control-Read-Encryption-Key"
#define HK_HKDF_CONTROL_WRITE_SALT "Control-Salt"
#define HK_HKDF_CONTROL_WRITE_INFO "Control-Write-Encryption-Key"
#define HK_HKDF_BROADCAST_ENCRYPTION_KEY_INFO "Broadcast-Encryption-Key"

esp_err_t hk_hkdf(hk_mem *key_in, hk_mem* key_out, const char* salt, const char* info);
esp_err_t hk_hkdf_with_given_size(hk_mem *key_in, hk_mem* key_out, size_t size, const char* salt, const char* info);
esp_err_t hk_hkdf_with_external_salt(hk_mem *key_in, hk_mem *key_out, hk_mem *salt, const char* info);
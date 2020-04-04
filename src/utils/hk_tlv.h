#pragma once

#include <stdlib.h>
#include "../include/hk_mem.h"
#include "hk_res.h"

enum hk_tlv_types //todo: move to usages
{
    HK_TLV_Method = 0,        // (integer) Method to use for pairing. See PairMethod
    HK_TLV_Identifier = 1,    // (UTF-8) Identifier for authentication
    HK_TLV_Salt = 2,          // (bytes) 16+ bytes of random salt
    HK_TLV_PublicKey = 3,     // (bytes) Curve25519, SRP public key or signed Ed25519 key
    HK_TLV_Proof = 4,         // (bytes) Ed25519 or SRP proof
    HK_TLV_EncryptedData = 5, // (bytes) Encrypted data with auth tag at end
    HK_TLV_State = 6,         // (integer) State of the pairing process. 1=M1, 2=M2, etc.
    HK_TLV_Error = 7,         // (integer) Error code. Must only be present if error code is
                              // not 0. See HK_TLV_ERROR
    HK_TLV_RetryDelay = 8,    // (integer) Seconds to delay until retrying a setup code
    HK_TLV_Certificate = 9,   // (bytes) X.509 Certificate
    HK_TLV_Signature = 10,    // (bytes) Ed25519
    HK_TLV_Permissions = 11,  // (integer) Bit value describing permissions of the controller
                              // being added.
                              // None (0x00): Regular user
                              // Bit 1 (0x01): Admin that is able to add and remove
                              // pairings against the accessory
    HK_TLV_FragmentData = 13, // (bytes) Non-last fragment of data. If length is 0,
                              // it's an ACK.
    HK_TLV_FragmentLast = 14, // (bytes) Last fragment of data
    HK_TLV_Separator = 0xff,
};

enum hk_tlv_errors
{
    HK_TLV_ERROR_Unknown = 1,        // Generic error to handle unexpected errors
    HK_TLV_ERROR_Authentication = 2, // Setup code or signature verification failed
    HK_TLV_ERROR_Backoff = 3,        // Client must look at the retry delay TLV item and
                                     // wait that many seconds before retrying
    HK_TLV_ERROR_MaxPeers = 4,       // Server cannot accept any more pairings
    HK_TLV_ERROR_MaxTries = 5,       // Server reached its maximum number of
                                     // authentication attempts
    HK_TLV_ERROR_Unavailable = 6,    // Server pairing method is unavailable
    HK_TLV_ERROR_Busy = 7,           // Server is busy and cannot accept a pairing
                                     // request at this time
};

typedef struct
{
    char type;
    char length;
    char *value;
} hk_tlv_t;

hk_tlv_t *hk_tlv_add_state(hk_tlv_t *tlv_list, enum hk_tlv_types type);
hk_tlv_t *hk_tlv_add_error(hk_tlv_t *tlv_list, enum hk_tlv_errors error);
hk_tlv_t *hk_tlv_add_str(hk_tlv_t *tlv_list, char type, char *data);
hk_tlv_t *hk_tlv_add_buffer(hk_tlv_t *tlv_list, char type, char *data, size_t length);
hk_tlv_t *hk_tlv_add_mem(hk_tlv_t *tlv_list, char type, hk_mem* mem);
hk_tlv_t *hk_tlv_add_uint8(hk_tlv_t *tlv_list, char type, uint8_t data);
hk_tlv_t *hk_tlv_add_uint16(hk_tlv_t *tlv_list, char type, uint16_t data);
hk_tlv_t *hk_tlv_add(hk_tlv_t *tlv_list, char type, hk_mem *data);
esp_err_t hk_tlv_get_mem_by_type(hk_tlv_t *tlv, char type, hk_mem *result);
hk_tlv_t *hk_tlv_get_tlv_by_type(hk_tlv_t *tlv_list, char type);
size_t hk_tlv_get_size(hk_tlv_t *tlv_list);
void hk_tlv_free(hk_tlv_t *tlv_list);

void hk_tlv_serialize(hk_tlv_t *tlv_list, hk_mem *result);
hk_tlv_t *hk_tlv_deserialize(hk_mem *data);
hk_tlv_t *hk_tlv_deserialize_buffer(char *data, size_t size);

void hk_tlv_log(const char *title, hk_tlv_t *tlv_list, bool with_value, bool formatted);

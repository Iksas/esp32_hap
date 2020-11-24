/**
 * @file hk_pair_tlvs.h
 *
 * TLV codes for pairing.
 */

#pragma once

#define HK_PAIR_TLV_METHOD          0x00 // (integer) Method to use for pairing. See PairMethod   
#define HK_PAIR_TLV_IDENTIFIER      0x01 // (UTF-8) Identifier for authentication 
#define HK_PAIR_TLV_SALT            0x02 // (bytes) 16+ bytes of random salt 
#define HK_PAIR_TLV_PUBLICKEY       0x03 // (bytes) Curve25519, SRP public key or signed Ed25519 key 
#define HK_PAIR_TLV_PROOF           0x04 // (bytes) Ed25519 or SRP proof 
#define HK_PAIR_TLV_ENCRYPTEDDATA   0x05 // (bytes) Encrypted data with auth tag at end 
#define HK_PAIR_TLV_STATE           0x06 // (integer) State of the pairing process. 1=M1, 2=M2, etc. 
#define HK_PAIR_TLV_ERROR           0x07 // (integer) Error code. 
#define HK_PAIR_TLV_RETRYDELAY      0x08 // (integer) Seconds to delay until retrying a setup code   
#define HK_PAIR_TLV_CERTIFICATE     0x09 // (bytes) X.509 Certificate    
#define HK_PAIR_TLV_SIGNATURE       0x0A // (bytes) Ed25519 
#define HK_PAIR_TLV_PERMISSIONS     0x0B // (integer) Bit value describing permissions of the controller being added.
#define HK_PAIR_TLV_FRAGMENTDATA    0x0C // (bytes) Non-last fragment of data. If length is 0, it's an ACK. 
#define HK_PAIR_TLV_FRAGMENTLAST    0x0D // (bytes) Last fragment of data 
#define HK_PAIR_TLV_SESSIONID       0x0E
#define HK_PAIR_TLV_FLAGS           0x13 
#define HK_PAIR_TLV_SEPARATOR       0xFF    

#define HK_PAIR_TLV_METHOD_RESUME   0x06

#define HK_PAIR_TLV_STATE_M1        0x01
#define HK_PAIR_TLV_STATE_M2        0x02 
#define HK_PAIR_TLV_STATE_M3        0x03
#define HK_PAIR_TLV_STATE_M4        0x04
#define HK_PAIR_TLV_STATE_M5        0x05
#define HK_PAIR_TLV_STATE_M6        0x06 

#define HK_PAIR_TLV_ERROR_UNKNOWN        0x01  // Generic error to handle unexpected errors
#define HK_PAIR_TLV_ERROR_AUTHENTICATION 0x02  // Setup code or signature verification failed
#define HK_PAIR_TLV_ERROR_BACKOFF        0x03  // Client must look at the retry delay TLV item and
                                               // wait that many seconds before retrying
#define HK_PAIR_TLV_ERROR_MAXPEERS       0x04  // Server cannot accept any more pairings
#define HK_PAIR_TLV_ERROR_MAXTRIES       0x05  // Server reached its maximum number of
                                               // authentication attempts
#define HK_PAIR_TLV_ERROR_UNAVAILABLE    0x06  // Server pairing method is unavailable
#define HK_PAIR_TLV_ERROR_BUSY           0x07  // Server is busy and cannot accept a pairing
                                               // request at this time
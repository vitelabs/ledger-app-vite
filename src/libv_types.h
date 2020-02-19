/*******************************************************************************
*   Vite Wallet for Ledger Nano S
*   (c) 2020 Vite Labs
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#ifndef LIBV_TYPES_H

#define LIBV_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "os_io_seproxyhal.h"

typedef struct {
    /** IO flags to reply with at the end of an APDU handler */
    uint8_t ioFlags;
    /** Length of the outgoing command */
    uint16_t outLength;
    /** Pointer to the output buffer for the response */
    uint8_t *buffer;
} libv_apdu_response_t;

typedef enum {
    LIBV_STATE_READY,
    LIBV_STATE_CONFIRM_ADDRESS,
    LIBV_STATE_CONFIRM_RECEIVE_SIGNATURE,
    LIBV_STATE_CONFIRM_SEND_SIGNATURE,
} libv_state_t;

typedef struct {
    char *prefix;
    uint8_t prefixLen;
} libv_address_formatter_t;

typedef struct {
    char *suffix;
    uint8_t suffixLen;
    uint8_t unitScale;
} libv_amount_formatter_t;

typedef uint8_t libv_private_key_t[32];
typedef uint8_t libv_public_key_t[32];
typedef uint8_t libv_hash_t[32];
typedef uint8_t libv_signature_t[64];
typedef uint8_t libv_block_type_t;
typedef uint8_t libv_address_t[21];
typedef uint8_t libv_hash_t[32];
typedef uint8_t libv_height_t[8];
typedef uint8_t libv_amount_t[32];
typedef uint8_t libv_token_id_t[10];
typedef uint8_t libv_data_hash_t[32];
typedef uint8_t libv_nonce_t[8];

typedef struct {
    libv_block_type_t type;
    libv_hash_t prev_hash;
    libv_height_t height;
    libv_address_t account_address;
} libv_block_prefix_data_t;

typedef struct {
    libv_amount_t fee;
    libv_nonce_t nonce;
} libv_block_suffix_data_t;

typedef struct {
    libv_hash_t from_hash;
} libv_receive_block_infix_data_t;

typedef struct {
    libv_address_t toAddress;
    libv_amount_t amount;
    libv_token_id_t tokenId;
    libv_data_hash_t dataHash;
} libv_send_block_infix_data_t;

#endif // LIBV_TYPES_H
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

#ifndef LIBV_APDU_SIGN_SEND_BLOCK_H

#define LIBV_APDU_SIGN_SEND_BLOCK_H

#include "libv_types.h"
#include "libv_helpers.h"
#include "blake2b.h"

typedef struct {
    uint8_t keyPath[MAX_BIP32_PATH_LENGTH];
    libv_public_key_t publicKey;
    libv_address_t toAddress;
    libv_token_id_t tokenId;
    libv_amount_t amount;
    libv_amount_t fee;
    libv_hash_t blockHash;
    uint8_t data[64]; // truncated
    uint64_t dataLen;
    bool dataIsNote;
    bool hasFee;
    libv_amount_formatter_t amountFmt;
} libv_apdu_sign_send_block_request_t;

uint16_t libv_apdu_sign_send_block(libv_apdu_response_t *resp);

#endif // LIBV_APDU_SIGN_SEND_BLOCK_H

/*******************************************************************************
*   Vite Wallet for Ledger Nano S
*   (c) 2020 Vite Labs
*   (c) 2016 Ledger
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

#ifndef LIBV_HELPERS_H

#define LIBV_HELPERS_H

#include "libv_types.h"

#define MAX_BIP32_PATH 10
#define MAX_BIP32_PATH_LENGTH (4 * MAX_BIP32_PATH) + 1

uint32_t libv_read_u32(uint8_t *buffer, bool be, bool skipSign);

void libv_write_hex_string(uint8_t *buffer, const uint8_t *bytes, size_t bytesLen);

size_t libv_user_raw_address_format(uint8_t *buffer, const libv_public_key_t publicKey);

size_t libv_user_address_format(uint8_t *buffer, const libv_public_key_t publicKey);

size_t libv_address_format(uint8_t *buffer, const libv_address_t rawAddress);

size_t libv_contract_address_name(uint8_t *buffer, const libv_address_t rawAddress);

void libv_derive_keypair(uint8_t *bip32Path,
                         libv_private_key_t out_privateKey,
                         libv_public_key_t out_publicKey);

/** Implement Java hashCode() equivalent hashing of data **/
uint32_t libv_simple_hash(uint8_t *data, size_t dataLen);

void libv_sign_hash(libv_signature_t signature,
                    const libv_hash_t hash,
                    const libv_private_key_t privateKey,
                    const libv_public_key_t publicKey);

size_t libv_hex_to_ascii(uint8_t *hex, size_t hexlen, uint8_t *ascii);

#endif

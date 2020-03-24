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

#include "libv_internal.h"
#include "libv_apdu_constants.h"
#include "ed25519.h"
#include "blake2b.h"
#include "libv_rom_variables.h"

#define LIBV_CURVE CX_CURVE_Ed25519
#define LIBV_SEED_KEY "ed25519 blake2b seed"

// Define some binary "literals" for the massive bit manipulation operation
// when converting public key to account string.
#define B_11111 31
#define B_01111 15
#define B_00111  7
#define B_00011  3
#define B_00001  1

uint32_t libv_read_u32(uint8_t *buffer, bool be,
                       bool skipSign) {
    uint8_t i;
    uint32_t result = 0;
    uint8_t shiftValue = (be ? 24 : 0);
    for (i = 0; i < 4; i++) {
        uint8_t x = (uint8_t)buffer[i];
        if ((i == 0) && skipSign) {
            x &= 0x7f;
        }
        result += ((uint32_t)x) << shiftValue;
        if (be) {
            shiftValue -= 8;
        } else {
            shiftValue += 8;
        }
    }
    return result;
}

void libv_write_hex_string(uint8_t *buffer, const uint8_t *bytes, size_t bytesLen) {
    uint32_t i;
    for (i = 0; i < bytesLen; i++) {
        buffer[2*i] = BASE16_ALPHABET[(bytes[i] >> 4) & 0xF];
        buffer[2*i+1] = BASE16_ALPHABET[bytes[i] & 0xF];
    }
}

// Extract a single-path component from the encoded bip32 path
uint32_t libv_bip32_get_component(uint8_t *path, uint8_t n) {
    uint8_t pathLength = path[0];
    if (n >= pathLength) {
        THROW(INVALID_PARAMETER);
    }
    return libv_read_u32(&path[1 + 4 * n], 1, 0);
}

size_t libv_user_raw_address_format(uint8_t *buffer, const libv_public_key_t publicKey) {
    uint8_t raw[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t userAddressSuffix = 0x00;
    black2b(publicKey, sizeof(libv_public_key_t), raw, sizeof(raw));
    os_memmove(buffer, raw, sizeof(raw));
    buffer += sizeof(raw);
    // user address has 0x00 suffix
    os_memmove(buffer, &userAddressSuffix, 1);
    return sizeof(raw)+1;
}

size_t libv_address_format(uint8_t *buffer, const libv_address_t rawAddress) {

    uint8_t check[5] = { 0, 0, 0, 0, 0 };
    uint8_t addressSuffix = rawAddress[20];

    uint8_t rawHexString[40];
    uint8_t checkHexString[10];

    black2b(rawAddress, 20, check, sizeof(check));

    if (addressSuffix != 0x00) {
        for (size_t i = 0; i < sizeof(check); i++)
        {
            check[i] = check[i]^0xff;
        }
        
    }

    libv_hex_to_ascii(rawAddress, 20, rawHexString);
    libv_hex_to_ascii(check, sizeof(check), checkHexString);

    // Write prefix
    uint8_t prefix[5] = { 'v', 'i', 't', 'e', '_' };
    os_memmove(buffer, prefix, sizeof(prefix));
    buffer += sizeof(prefix);
    // Write raw
    os_memmove(buffer, rawHexString, sizeof(rawHexString));
    buffer += sizeof(rawHexString);
    // Write check
    os_memmove(buffer, checkHexString, sizeof(checkHexString));
    buffer += sizeof(checkHexString);

    return sizeof(prefix) + sizeof(rawHexString) + sizeof(checkHexString);   
}

size_t libv_user_address_format(uint8_t *buffer, const libv_public_key_t publicKey) {
    libv_address_t rawAddress;
    libv_user_raw_address_format(rawAddress, publicKey);
    return libv_address_format(buffer, rawAddress);
}

size_t libv_contract_address_name(uint8_t *buffer, const libv_address_t rawAddress) {

    uint8_t addressSuffix = rawAddress[20];

    if (addressSuffix != 0x01) {
        return 0;
    }

    for (size_t i = 0; i < 19; i++) {
        if (rawAddress[i] != 0x00) {
            return 0;
        }
    }
    
    uint8_t index = rawAddress[19];
    if (index >= CONTRACT_INFO_ARRAY_LEN) {
        return 0;
    }

    CONTRACT_INFO contractInfo = CONTRACT_INFO_ARRAY[index];
    size_t len = strnlen(contractInfo.addressName, CONTRACT_ADDRESS_NAME_MAX_LEN);

    if (len == 0) {
        return 0;
    }

    os_memmove(buffer, contractInfo.addressName, len);
    return len;  
}

size_t libv_token_id_format(uint8_t *buffer,
                           const libv_token_id_t tokenId) {
    uint8_t check[2] = { 0, 0, 0, 0, 0 };

    uint8_t rawHexString[20];
    uint8_t checkHexString[4];

    black2b(tokenId, sizeof(libv_token_id_t), check, sizeof(check));

    libv_hex_to_ascii(tokenId, sizeof(libv_token_id_t), rawHexString);
    libv_hex_to_ascii(check, sizeof(check), checkHexString);

    // Write prefix
    uint8_t prefix[4] = { 't', 't', 'i', '_' };
    os_memmove(buffer, prefix, sizeof(prefix));
    buffer += sizeof(prefix);
    // Write raw
    os_memmove(buffer, rawHexString, sizeof(rawHexString));
    buffer += sizeof(rawHexString);
    // Write check
    os_memmove(buffer, checkHexString, sizeof(checkHexString));
    buffer += sizeof(checkHexString);

    return sizeof(prefix) + sizeof(rawHexString) + sizeof(checkHexString);
}

void libv_amount_format(const libv_amount_formatter_t *fmt,
                        char *dest, size_t destLen,
                        const libv_amount_t balance) {

    char buf[AMOUNT_TEXT_MAX_LEN];
    libv_amount_t num;

    os_memset(buf, 0, sizeof(buf));
    os_memmove(num, balance, sizeof(num));

    size_t end = sizeof(buf);
    end -= 1; // '\0' NULL terminator
    size_t start = end;

    // Convert the balance into a string by dividing by 10 until
    // zero is reached
    uint16_t r;
    uint16_t d;
    do {
        r = num[0];
        d = r / 10; r = ((r - d * 10) << 8) + num[1]; num[0] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[2]; num[1] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[3]; num[2] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[4]; num[3] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[5]; num[4] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[6]; num[5] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[7]; num[6] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[8]; num[7] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[9]; num[8] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[10]; num[9] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[11]; num[10] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[12]; num[11] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[13]; num[12] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[14]; num[13] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[15]; num[14] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[16]; num[15] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[17]; num[16] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[18]; num[17] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[19]; num[18] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[20]; num[19] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[21]; num[20] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[22]; num[21] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[23]; num[22] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[24]; num[23] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[25]; num[24] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[26]; num[25] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[27]; num[26] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[28]; num[27] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[29]; num[28] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[30]; num[29] = d;
        d = r / 10; r = ((r - d * 10) << 8) + num[31]; num[30] = d;
        d = r / 10; r = r - d * 10; num[31] = d;
        buf[--start] = '0' + r;
    } while (num[0]  || num[1]  || num[2]  || num[3]  ||
             num[4]  || num[5]  || num[6]  || num[7]  ||
             num[8]  || num[9]  || num[10] || num[11] ||
             num[12] || num[13] || num[14] || num[15] ||
             num[16] || num[17] || num[18] || num[19] || 
             num[20] || num[21] || num[22] || num[23] ||
             num[24] || num[25] || num[26] || num[27] ||
             num[28] || num[29] || num[30] || num[31] );

    // Assign the location for the decimal point
    size_t point = end - 1 - fmt->unitScale;
    // Make sure that the number is zero padded until the point location
    while (start > point) {
        buf[--start] = '0';
    }
    // Move digits before the point one place to the left
    for (size_t i = start; i <= point; i++) {
        buf[i-1] = buf[i];
    }
    start -= 1;
    // It's safe to write out the point now
    if (point != end) {
        buf[point] = '.';
    }

    // Remove as many zeros from the fractional part as possible
    while (end > point && (buf[end-1] == '0' || buf[end-1] == '.')) {
        end -= 1;
    }
    buf[end] = '\0';

    if (fmt->suffixLen > 0) {
        // Append the unique symbol
        buf[--start] = ' ';
        start -= fmt->suffixLen;
        os_memmove(buf + start, fmt->suffix, fmt->suffixLen);
    }

    // Copy the result to the destination buffer
    os_memmove(dest, buf + start, MIN(destLen - 1, end - start + 1));
    dest[destLen - 1] = '\0';
}

void libv_derive_keypair(uint8_t *bip32Path,
                         libv_private_key_t out_privateKey,
                         libv_public_key_t out_publicKey) {
    uint32_t bip32PathInt[MAX_BIP32_PATH];
    uint8_t chainCode[32];
    uint8_t bip32PathLength;
    uint8_t i;
    const uint8_t bip32PrefixLength = 2;

    bip32PathLength = bip32Path[0];
    if (bip32PathLength > MAX_BIP32_PATH) {
        THROW(INVALID_PARAMETER);
    }
    bip32Path++;
    for (i = 0; i < bip32PathLength; i++) {
        bip32PathInt[i] = libv_read_u32(bip32Path, 1, 0);
        bip32Path += 4;
    }
    // Verify that the prefix is the allowed prefix
    if (bip32PathLength < bip32PrefixLength) {
        THROW(INVALID_PARAMETER);
    }

    os_perso_derive_node_bip32_seed_key(
        HDW_ED25519_SLIP10, LIBV_CURVE,
        bip32PathInt, bip32PathLength,
        out_privateKey, chainCode,
        (unsigned char *)LIBV_SEED_KEY, sizeof(LIBV_SEED_KEY)
    );
    os_memset(chainCode, 0, sizeof(chainCode));

    if (out_publicKey != NULL) {
        ed25519_publickey(out_privateKey, out_publicKey);
    }
}

uint32_t libv_simple_hash(uint8_t *data, size_t dataLen) {
    uint32_t result = 5;
    for (size_t i = 0; i < dataLen; i++) {
        result = 29 * result + data[i];
    }
    return result;
}

void libv_sign_hash(libv_signature_t signature,
                    const libv_hash_t hash,
                    const libv_private_key_t privateKey,
                    const libv_public_key_t publicKey) {
    ed25519_sign(
        hash, sizeof(libv_hash_t),
        privateKey, publicKey,
        signature);
}

size_t libv_hex_to_ascii(uint8_t *hex, size_t hexlen, uint8_t *ascii) {
    for (int i = 0; i < hexlen; i++) {
        uint8_t high = (hex[i]>>4)&0x0f;
        uint8_t low = hex[i]&0x0f;
        *ascii = high > 9 ? ('a'+high-10) : ('0'+high);
        ascii++;
        *ascii = low > 9 ? ('a'+low-10) : ('0'+low);
        ascii++;
    }

    return hexlen * 2;
}
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

#include "libv_internal.h"
#include "libv_apdu_constants.h"
#include "libv_apdu_sign_send_block.h"
#include "libv_bagl.h"

#define P1_USE_CACHED_DATA 0x01
#define P1_NOT_USE_CACHED_DATA 0x02
#define P2_UNUSED 0x00

uint16_t libv_apdu_sign_send_block_output(libv_apdu_response_t *resp, libv_apdu_sign_send_block_request_t *req);

uint16_t libv_apdu_sign_send_block(libv_apdu_response_t *resp) {
    libv_apdu_sign_send_block_request_t req;
    libv_private_key_t privateKey;
    libv_block_prefix_data_t block_prefix;
    libv_send_block_infix_data_t block_infix;
    libv_block_suffix_data_t block_suffix;

    uint8_t *ptr;
    uint8_t len;
    uint8_t source[sizeof(libv_block_prefix_data_t) + sizeof(libv_send_block_infix_data_t) + sizeof(libv_block_suffix_data_t)];

    switch (G_io_apdu_buffer[ISO_OFFSET_P1]) {
    case P1_USE_CACHED_DATA:
    case P1_NOT_USE_CACHED_DATA:
        break;
    default:
        return LIBV_SW_INCORRECT_P1_P2;
    }

    switch (G_io_apdu_buffer[ISO_OFFSET_P2]) {
    case P2_UNUSED:
        break;
    default:
        return LIBV_SW_INCORRECT_P1_P2;
    }

    // Verify the minimum size
    if (G_io_apdu_buffer[ISO_OFFSET_LC] < 156) {
        return LIBV_SW_INCORRECT_LENGTH;
    }

    bool hasCachedData = (G_io_apdu_buffer[ISO_OFFSET_P1] == P1_USE_CACHED_DATA);

    ptr = G_io_apdu_buffer + ISO_OFFSET_CDATA;
    len = 1 + (*ptr) * 4;
    os_memmove(req.keyPath, ptr, MIN(len, sizeof(req.keyPath)));
    ptr += len;

    if (!os_global_pin_is_validated()) {
        return LIBV_SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    // Make sure that we're not about to interrupt another operation
    if (libv_context_D.state != LIBV_STATE_READY) {
        return LIBV_SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // Derive public key for hashing
    libv_derive_keypair(req.keyPath, privateKey, req.publicKey);
    os_memset(privateKey, 0, sizeof(privateKey)); // sanitise private key

    // Parse input data
    block_prefix.type = 0x02;
    libv_user_raw_address_format(block_prefix.account_address, req.publicKey);

    len = sizeof(block_prefix.prev_hash);
    os_memmove(block_prefix.prev_hash, ptr, len);
    ptr += len;

    len = sizeof(block_prefix.height);
    os_memmove(block_prefix.height, ptr, len);
    ptr += len;

    len = sizeof(block_infix.toAddress);
    os_memmove(block_infix.toAddress, ptr, len);
    ptr += len;

    len = sizeof(block_infix.amount);
    os_memmove(block_infix.amount, ptr, len);
    ptr += len;

    len = sizeof(block_infix.tokenId);
    os_memmove(block_infix.tokenId, ptr, len);
    ptr += len;

    len = sizeof(block_suffix.fee);
    os_memmove(block_suffix.fee, ptr, len);
    ptr += len;

    len = sizeof(block_suffix.nonce);
    os_memmove(block_suffix.nonce, ptr, len);
    ptr += len;

    uint64_t dataLen = G_io_apdu_buffer[ISO_OFFSET_LC] - (ptr - (G_io_apdu_buffer + ISO_OFFSET_CDATA));

    os_memmove(req.toAddress, block_infix.toAddress, sizeof(req.toAddress));
    os_memmove(req.tokenId, block_infix.tokenId, sizeof(req.tokenId));
    os_memmove(req.amount, block_infix.amount, sizeof(req.amount));

    if (hasCachedData) {
        libv_apdu_cache_send_block_data_request_t cached = libv_context_D.cachedSendBlockData;
        if (dataLen > 0) {
            blake2b_update(&cached.c, ptr, dataLen);
            blake2b_final(&cached.c, block_infix.dataHash);
            os_memmove(req.data, cached.data, MIN(cached.dataLen, 64));
            if (cached.dataLen <64) {
                os_memmove(req.data + cached.dataLen, ptr, MIN(dataLen, 64 - cached.dataLen));
            }
            req.dataLen = cached.dataLen + dataLen;
        } else {
            blake2b_final(&cached.c, block_infix.dataHash);
            os_memmove(req.data, cached.data, MIN(cached.dataLen, 64));
            req.dataLen = cached.dataLen;
        }
        req.dataIsNote = cached.dataIsNote;
    } else {
        if (dataLen > 0) {
            blake2b_ctx c = libv_context_D.cachedSendBlockData.c;
            blake2b_init(&c, sizeof(libv_hash_t), NULL, 0);
            blake2b_update(&c, ptr, dataLen);
            blake2b_final(&c, block_infix.dataHash);
            os_memmove(req.data, ptr, MIN(dataLen, 64));
            req.dataLen = dataLen; 
        } else {
            req.dataLen = 0;
        }
    }

    if (req.dataLen > 0) {
        if (req.dataIsNote) {
            for (size_t i = 0; i < dataLen; i++)
            {
                if (*(ptr + i) < 32 || *(ptr + i) > 126) {
                    req.dataIsNote = false;
                    break;
                }
            }
        }
    } else {
        req.dataIsNote = false;
    }

    // Parse source
    ptr = source;

    len = sizeof(libv_block_prefix_data_t);
    os_memmove(ptr, &block_prefix, len);
    ptr += len;

    if (req.dataLen > 0) {
        len = sizeof(libv_send_block_infix_data_t);
    } else {
        len = sizeof(libv_send_block_infix_data_t) - sizeof(libv_data_hash_t);
    }

    os_memmove(ptr, &block_infix, len);
    ptr += len;

    len = sizeof(libv_block_suffix_data_t);
    os_memmove(ptr, &block_suffix, len);
    ptr += len;

    len = ptr - source;
    black2b(source, len, req.blockHash, sizeof(req.blockHash));

    // Update app state to confirm the address
    libv_context_D.state = LIBV_STATE_CONFIRM_SEND_SIGNATURE;
    os_memmove(&libv_context_D.stateData.signSendBlockRequest, &req, sizeof(req));
    os_memset(&req, 0, sizeof(req)); // sanitise request data
    app_apply_state();

    resp->ioFlags |= IO_ASYNCH_REPLY;
    return LIBV_SW_OK;
}



uint16_t libv_apdu_sign_send_block_output(libv_apdu_response_t *resp, libv_apdu_sign_send_block_request_t *req) {
    libv_private_key_t privateKey;
    libv_signature_t signature;
    uint8_t *outPtr = resp->buffer;

    // Derive key and sign the block
    libv_derive_keypair(req->keyPath, privateKey, NULL);
    libv_sign_hash(signature, req->blockHash, privateKey, req->publicKey);
    os_memset(privateKey, 0, sizeof(privateKey));

    // Output block hash
    os_memmove(outPtr, req->blockHash, sizeof(req->blockHash));
    outPtr += sizeof(req->blockHash);

    // Output signature
    os_memmove(outPtr, signature, sizeof(signature));
    outPtr += sizeof(signature);

    resp->outLength = outPtr - resp->buffer;

    return LIBV_SW_OK;
}

void libv_bagl_confirm_sign_send_block_callback(bool confirmed) {
    libv_apdu_sign_send_block_request_t *req = &libv_context_D.stateData.signSendBlockRequest;

    uint16_t statusWord;
    libv_apdu_response_t resp;
    resp.buffer = libv_async_buffer_D;
    resp.outLength = 0;
    resp.ioFlags = 0;

    if (confirmed) {
        statusWord = libv_apdu_sign_send_block_output(&resp, req);
    } else {
        statusWord = LIBV_SW_CONDITIONS_OF_USE_NOT_SATISFIED;
    }
    os_memset(req, 0, sizeof(req)); // sanitise request data
    app_async_response(&resp, statusWord);
}
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
#include "libv_apdu_sign_receive_block.h"
#include "libv_bagl.h" 
#include "libv_debug.h"

#define P1_UNUSED 0x00
#define P2_UNUSED 0x00

uint16_t libv_apdu_sign_receive_block_output(libv_apdu_response_t *resp, libv_apdu_sign_receive_block_request_t *req);

uint16_t libv_apdu_sign_receive_block(libv_apdu_response_t *resp) {
    libv_apdu_sign_receive_block_request_t req;
    libv_private_key_t privateKey;
    libv_block_prefix_data_t block_prefix;
    libv_receive_block_infix_data_t block_infix;
    libv_block_suffix_data_t block_suffix;

    uint8_t *inPtr;
    uint8_t readLen;
    uint8_t source[sizeof(libv_block_prefix_data_t) + sizeof(libv_receive_block_infix_data_t) + sizeof(libv_block_suffix_data_t)];

    switch (G_io_apdu_buffer[ISO_OFFSET_P1]) {
    case P1_UNUSED:
        break;
    default:
        return LIBV_SW_INCORRECT_P1_P2;
    }

    switch (G_io_apdu_buffer[ISO_OFFSET_P2]) {
    case P2_UNUSED:
    #ifdef LIBV_ENABLE_DEBUG
    case P2_DEBUG:
    #endif
        break;
    default:
        return LIBV_SW_INCORRECT_P1_P2;
    }

    // Verify the minimum size
    if (G_io_apdu_buffer[ISO_OFFSET_LC] < 93) {
        return LIBV_SW_INCORRECT_LENGTH;
    }

    inPtr = G_io_apdu_buffer + ISO_OFFSET_CDATA;
    readLen = 1 + (*inPtr) * 4;
    os_memmove(req.keyPath, inPtr, MIN(readLen, sizeof(req.keyPath)));
    inPtr += readLen;

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
    block_prefix.type = 0x04;
    libv_user_raw_address_format(block_prefix.account_address, req.publicKey);

    readLen = sizeof(block_prefix.prev_hash);
    os_memmove(block_prefix.prev_hash, inPtr, readLen);
    inPtr += readLen;

    readLen = sizeof(block_prefix.height);
    os_memmove(block_prefix.height, inPtr, readLen);
    inPtr += readLen;

    readLen = sizeof(block_infix.from_hash);
    os_memmove(block_infix.from_hash, inPtr, readLen);
    inPtr += readLen;

    os_memset(block_suffix.fee, 0, sizeof(block_suffix.fee));

    readLen = sizeof(block_suffix.nonce);
    os_memmove(block_suffix.nonce, inPtr, readLen);

    // for ui
    os_memmove(req.fromHash, block_infix.from_hash, sizeof(req.fromHash));

    // Parse source
    inPtr = source;

    readLen = sizeof(libv_block_prefix_data_t);
    os_memmove(inPtr, &block_prefix, readLen);
    inPtr += readLen;

    readLen = sizeof(libv_receive_block_infix_data_t);
    os_memmove(inPtr, &block_infix, readLen);
    inPtr += readLen;

    readLen = sizeof(libv_block_suffix_data_t);
    os_memmove(inPtr, &block_suffix, readLen);

    #ifdef LIBV_ENABLE_DEBUG
    if (G_io_apdu_buffer[ISO_OFFSET_P2] == P2_DEBUG) {
        return libv_apdu_test_output(resp, source, sizeof(source));
    }
    #endif

    black2b(source, sizeof(source), req.blockHash, sizeof(req.blockHash));

    // When auto receive is enabled, skip the prompt
    if (N_libv.autoReceive) {
        uint16_t statusWord = libv_apdu_sign_receive_block_output(resp, &req);
        os_memset(&req, 0, sizeof(req)); // sanitise request data
        return statusWord;
    } else {
        // Update app state to confirm the address
        libv_context_D.state = LIBV_STATE_CONFIRM_RECEIVE_SIGNATURE;
        os_memmove(&libv_context_D.stateData.signReceiveBlockRequest, &req, sizeof(req));
        os_memset(&req, 0, sizeof(req)); // sanitise request data
        app_apply_state();

        resp->ioFlags |= IO_ASYNCH_REPLY;
        return LIBV_SW_OK;
    }
}

uint16_t libv_apdu_sign_receive_block_output(libv_apdu_response_t *resp, libv_apdu_sign_receive_block_request_t *req) {
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

void libv_bagl_confirm_sign_receive_block_callback(bool confirmed) {
    libv_apdu_sign_receive_block_request_t *req = &libv_context_D.stateData.signReceiveBlockRequest;

    uint16_t statusWord;
    libv_apdu_response_t resp;
    resp.buffer = libv_async_buffer_D;
    resp.outLength = 0;
    resp.ioFlags = 0;

    if (confirmed) {
        statusWord = libv_apdu_sign_receive_block_output(&resp, req);
    } else {
        statusWord = LIBV_SW_CONDITIONS_OF_USE_NOT_SATISFIED;
    }
    os_memset(req, 0, sizeof(req)); // sanitise request data
    app_async_response(&resp, statusWord);
}

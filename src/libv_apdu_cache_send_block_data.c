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
#include "libv_apdu_cache_send_block_data.h"
#include "libv_bagl.h"

#define P1_FIRST_DATA 0x01
#define P1_OTHER_DATA 0x02
#define P2_UNUSED 0x00

uint16_t libv_apdu_cache_send_block_data_output(libv_apdu_response_t *resp, libv_apdu_cache_send_block_data_request_t *req);

uint16_t libv_apdu_cache_send_block_data(libv_apdu_response_t *resp) {
    libv_apdu_cache_send_block_data_request_t req;
    uint8_t keyPath[MAX_BIP32_PATH_LENGTH];
    libv_private_key_t privateKey;
    uint8_t *ptr;
    uint8_t len;

    switch (G_io_apdu_buffer[ISO_OFFSET_P1]) {
    case P1_FIRST_DATA:
    case P1_OTHER_DATA:
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

    // Verify the size
    if (G_io_apdu_buffer[ISO_OFFSET_LC] == 0 || G_io_apdu_buffer[ISO_OFFSET_LC] > 224) {
        return LIBV_SW_INCORRECT_LENGTH;
    }

    if (!os_global_pin_is_validated()) {
        return LIBV_SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    // Make sure that we're not about to interrupt another operation
    if (libv_context_D.state != LIBV_STATE_READY) {
        return LIBV_SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    if (G_io_apdu_buffer[ISO_OFFSET_P1] == P1_FIRST_DATA) {
        blake2b_init(&req.c, sizeof(libv_hash_t), NULL, 0);
        req.dataLen = 0;
        req.dataIsNote = true;
    } else {
        os_memmove(&req, &libv_context_D.cachedSendBlockData, sizeof(libv_apdu_cache_send_block_data_request_t));
    }

    ptr = G_io_apdu_buffer + ISO_OFFSET_CDATA;
    len = G_io_apdu_buffer[ISO_OFFSET_LC];

    blake2b_update(&req.c, ptr, len);
    if (req.dataLen < 64) {
        os_memmove(req.data + req.dataLen, ptr, MIN(len, 64 - req.dataLen));
    }
    req.dataLen += len;

    if (req.dataIsNote) {
        for (size_t i = 0; i < len; i++)
        {
            if (*(ptr + i) < 32 || *(ptr + i) > 126) {
                req.dataIsNote = false;
                break;
            }
        }
    }

    uint16_t statusWord = libv_apdu_cache_send_block_data_output(resp, &req);
    os_memset(&req, 0, sizeof(req)); // sanitise request data
    return statusWord;
}

uint16_t libv_apdu_cache_send_block_data_output(libv_apdu_response_t *resp, libv_apdu_cache_send_block_data_request_t *req) {
    // Copy the data over to the cache
    os_memmove(&libv_context_D.cachedSendBlockData, req, sizeof(libv_apdu_cache_send_block_data_request_t));

    #ifdef LIBV_ENABLE_DEBUG
    return libv_apdu_test_output(resp, libv_context_D.cachedSendBlockData.data, MIN(libv_context_D.cachedSendBlockData.dataLen, 64));
    #endif
    return LIBV_SW_OK;
}

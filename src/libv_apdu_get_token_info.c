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
#include "libv_apdu_get_token_info.h"
#include "libv_apdu_constants.h"
#include "libv_helpers.h"

#define P1_UNUSED 0x00
#define P2_UNUSED 0x00

uint16_t libv_apdu_get_token_info_output(libv_apdu_response_t *resp, TOKEN_INFO tokenInfo);

uint16_t libv_apdu_get_token_info(libv_apdu_response_t *resp) {
    switch (G_io_apdu_buffer[ISO_OFFSET_P1]) {
    case P1_UNUSED:
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

    if (G_io_apdu_buffer[ISO_OFFSET_LC] != 2) {
        return LIBV_SW_INCORRECT_LENGTH;
    }

    uint16_t index = (((uint16_t)G_io_apdu_buffer[ISO_OFFSET_CDATA]) << 8) + G_io_apdu_buffer[ISO_OFFSET_CDATA + 1];

    if (index >= TOKEN_INFO_ARRAY_LEN) {
        return LIBV_SW_ARRAY_OUT_OF_BOUNDS;
    }

    TOKEN_INFO tokenInfo = TOKEN_INFO_ARRAY[index];
    return libv_apdu_get_token_info_output(resp, tokenInfo);
}

uint16_t libv_apdu_get_token_info_output(libv_apdu_response_t *resp, TOKEN_INFO tokenInfo) {
    uint8_t *outPtr = resp->buffer;
    uint8_t length;

    length = libv_token_id_format(outPtr, tokenInfo.tokenId);;
    outPtr += length;

    length = 1;
    os_memmove(outPtr, &tokenInfo.unitScale, length);
    outPtr += length;

    length = strnlen(tokenInfo.suffix, sizeof(tokenInfo.suffix));
    os_memmove(outPtr, tokenInfo.suffix, length);
    outPtr += length;

    resp->outLength = outPtr - resp->buffer;

    return LIBV_SW_OK;
}

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
#include "libv_apdu_test_amount_text.h"
#include "libv_apdu_constants.h"
#include "libv_helpers.h"

#define P1_UNUSED 0x00
#define P2_UNUSED 0x00

uint16_t libv_apdu_test_amount_text_output(libv_apdu_response_t *resp, libv_amount_formatter_t amountFmt, libv_amount_t amount);

uint16_t libv_apdu_test_amount_text(libv_apdu_response_t *resp) {
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

    if (G_io_apdu_buffer[ISO_OFFSET_LC] != 0x2A) {
        return LIBV_SW_INCORRECT_LENGTH;
    }

    uint8_t *ptr = G_io_apdu_buffer + ISO_OFFSET_CDATA;
    uint8_t len;
    libv_amount_t amount;
    libv_token_id_t tokenId;

    len = sizeof(libv_amount_t);
    os_memmove(amount, ptr, len);
    ptr += len;

    len = sizeof(libv_token_id_t);
    os_memmove(tokenId, ptr, len);
    ptr += len;


    libv_amount_formatter_t amountFmt;
    os_memset(&amountFmt, 0, sizeof(libv_amount_formatter_t));

    for (size_t i = 0; i < TOKEN_INFO_ARRAY_LEN; i++)
    {
        TOKEN_INFO info = TOKEN_INFO_ARRAY[i];
        if (os_memcmp(tokenId, info.tokenId, sizeof(libv_token_id_t)) == 0) {
            amountFmt.suffix = info.suffix;
            amountFmt.suffixLen = strnlen(info.suffix, sizeof(info.suffix));
            amountFmt.unitScale = info.unitScale;
            break;
        }
    }

    return libv_apdu_test_amount_text_output(resp, amountFmt, amount);
}

uint16_t libv_apdu_test_amount_text_output(libv_apdu_response_t *resp, libv_amount_formatter_t amountFmt, libv_amount_t amount) {
    libv_amount_format(&amountFmt, resp->buffer, 100, amount);
    resp->outLength = strnlen(resp->buffer, 100);
    return LIBV_SW_OK;
}

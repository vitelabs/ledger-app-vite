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
#include "libv_apdu_get_app_conf.h"
#include "libv_apdu_constants.h"

#define P1_UNUSED 0x00
#define P2_UNUSED 0x00

uint16_t libv_apdu_get_app_conf_output(libv_apdu_response_t *resp);

uint16_t libv_apdu_get_app_conf(libv_apdu_response_t *resp) {
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

    if (G_io_apdu_buffer[ISO_OFFSET_LC] > 0x00) {
        return LIBV_SW_INCORRECT_LENGTH;
    }

    return libv_apdu_get_app_conf_output(resp);
}

uint16_t libv_apdu_get_app_conf_output(libv_apdu_response_t *resp) {
    uint8_t *outPtr = resp->buffer;
    uint8_t length;

    // Output app version
    *outPtr = APP_MAJOR_VERSION;
    outPtr += 1;
    *outPtr = APP_MINOR_VERSION;
    outPtr += 1;
    *outPtr = APP_PATCH_VERSION;
    outPtr += 1;

    // Output builtin token count
    *outPtr = (TOKEN_INFO_ARRAY_LEN >> 8) & 0xff;
    outPtr += 1;
    *outPtr = TOKEN_INFO_ARRAY_LEN & 0xff;
    outPtr += 1;

    resp->outLength = outPtr - resp->buffer;

    return LIBV_SW_OK;
}

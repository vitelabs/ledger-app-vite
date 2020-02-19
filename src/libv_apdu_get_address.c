/*******************************************************************************
*   Vite Wallet for Ledger Nano S
*   (c) 2020 Vite Labs
*   (c) 2018 Mart Roosmaa
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
#include "libv_apdu_get_address.h"
#include "libv_apdu_constants.h"

#include "libv_bagl.h"

#define P1_NO_DISPLAY 0x00
#define P1_DISPLAY 0x01

#define P2_UNUSED 0x00

uint16_t libv_apdu_get_address_output(libv_apdu_response_t *resp, libv_apdu_get_address_request_t *req);

uint16_t libv_apdu_get_address(libv_apdu_response_t *resp) {
    libv_apdu_get_address_request_t req;
    libv_private_key_t privateKey;

    uint8_t *keyPathPtr;
    bool display = (G_io_apdu_buffer[ISO_OFFSET_P1] == P1_DISPLAY);

    switch (G_io_apdu_buffer[ISO_OFFSET_P1]) {
    case P1_NO_DISPLAY:
    case P1_DISPLAY:
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

    if (G_io_apdu_buffer[ISO_OFFSET_LC] < 0x01) {
        return LIBV_SW_INCORRECT_LENGTH;
    }
    keyPathPtr = G_io_apdu_buffer + ISO_OFFSET_CDATA;

    if (!os_global_pin_is_validated()) {
        return LIBV_SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    // Make sure that we're not about to interrupt another operation
    if (display && libv_context_D.state != LIBV_STATE_READY) {
        return LIBV_SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // Retrieve the public key for the path
    libv_derive_keypair(keyPathPtr, privateKey, req.publicKey);
    os_memset(privateKey, 0, sizeof(privateKey)); // sanitise private key

    if (display) {
        // Update app state to confirm the address
        libv_context_D.state = LIBV_STATE_CONFIRM_ADDRESS;
        os_memmove(&libv_context_D.stateData.getAddressRequest, &req, sizeof(req));
        os_memset(&req, 0, sizeof(req)); // sanitise request data
        app_apply_state();

        resp->ioFlags |= IO_ASYNCH_REPLY;
        return LIBV_SW_OK;
    } else {
        uint16_t statusWord = libv_apdu_get_address_output(resp, &req);
        os_memset(&req, 0, sizeof(req)); // sanitise request data
        return statusWord;
    }
}

uint16_t libv_apdu_get_address_output(libv_apdu_response_t *resp, libv_apdu_get_address_request_t *req) {
    uint8_t length;
    uint8_t *outPtr = resp->buffer;

    // Output raw public key
    length = sizeof(req->publicKey);
    os_memmove(outPtr, req->publicKey, length);
    outPtr += length;

    // Encode & output account address
    length = libv_user_address_format(outPtr, req->publicKey);
    outPtr += length;

    resp->outLength = outPtr - resp->buffer;

    return LIBV_SW_OK;
}

void libv_bagl_display_address_callback(bool confirmed) {
    libv_apdu_get_address_request_t *req = &libv_context_D.stateData.getAddressRequest;

    uint16_t statusWord;
    libv_apdu_response_t resp;
    resp.buffer = libv_async_buffer_D;
    resp.outLength = 0;
    resp.ioFlags = 0;

    if (confirmed) {
        statusWord = libv_apdu_get_address_output(&resp, req);
    } else {
        statusWord = LIBV_SW_CONDITIONS_OF_USE_NOT_SATISFIED;
    }
    os_memset(req, 0, sizeof(req)); // sanitise request data
    app_async_response(&resp, statusWord);
}

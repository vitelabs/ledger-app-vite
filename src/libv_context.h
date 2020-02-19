/*******************************************************************************
*   Vite Wallet for Ledger Nano S
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

#ifndef LIBV_CONTEXT_H

#define LIBV_CONTEXT_H

#include "os.h"
#include "libv_types.h"
#include "libv_secure_value.h"
#include "libv_apdu_get_address.h"
#include "libv_apdu_sign_receive_block.h"
#include "libv_apdu_cache_send_block_data.h"
#include "libv_apdu_sign_send_block.h"

typedef struct {
    /** Flag if dongle has been halted */
    secu8 halted;

    /** Length of the incoming command */
    uint16_t inLength;

    /** Primary response for synchronous APDUs **/
    libv_apdu_response_t response;

    /** Cached send block data for sign send block **/
    libv_apdu_cache_send_block_data_request_t cachedSendBlockData;

    /** State determines the application state (UX displayed, etc).
        This is also used to enforce only single confirmation
        prompt. **/
    libv_state_t state;
    union {
        // when LIBV_STATE_READY
        libv_apdu_response_t asyncResponse;
        // when LIBV_STATE_CONFIRM_ADDRESS
        libv_apdu_get_address_request_t getAddressRequest;
        // when LIBV_STATE_CONFIRM_RECEIVE_SIGNATURE
        libv_apdu_sign_receive_block_request_t signReceiveBlockRequest;
        // whene LIBV_STATE_CONFIRM_SEND_SIGNATURE
        libv_apdu_sign_send_block_request_t signSendBlockRequest;
    } stateData;

#ifdef HAVE_IO_U2F
    /** U2F async request hash that is used to keep track and hook back
        into an async operation. **/
    uint32_t u2fRequestHash;
    /** U2F timeout tracker, once zero the active connection is dropped
        appropriate status code. **/
    uint16_t u2fTimeout;
#endif // HAVE_IO_U2F

} libv_context_t;

void libv_context_init(void);
void libv_context_move_async_response(void);

#endif

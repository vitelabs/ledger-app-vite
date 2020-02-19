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

#ifndef LIBV_APDU_GET_ADDRESS_H

#define LIBV_APDU_GET_ADDRESS_H

#include "libv_types.h"

typedef struct {
    libv_public_key_t publicKey;
} libv_apdu_get_address_request_t;

uint16_t libv_apdu_get_address(libv_apdu_response_t *resp);

#endif // LIBV_APDU_GET_ADDRESS_H

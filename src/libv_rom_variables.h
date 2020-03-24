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

#ifndef LIBV_ROM_VARIABLES_H

#define LIBV_ROM_VARIABLES_H

#include "libv_internal.h"

#define DISPATCHER_APDUS 7

typedef uint16_t (*apduProcessingFunction)(libv_apdu_response_t *resp);

extern uint8_t const DISPATCHER_CLA[DISPATCHER_APDUS];
extern uint8_t const DISPATCHER_INS[DISPATCHER_APDUS];
extern bool const DISPATCHER_DATA_IN[DISPATCHER_APDUS];
extern apduProcessingFunction const DISPATCHER_FUNCTIONS[DISPATCHER_APDUS];

#define LIBV_ACCOUNT_STRING_BASE_LEN 55

extern uint8_t const BASE16_ALPHABET[16];

#define TOKEN_UNIQUE_SYMBOL_MAX_LEN (10+1+3+1) // VITE-000\0
// MaxUInt256 = 1.1579209e+77 (78 digits)
#define AMOUNT_TEXT_MAX_LEN (TOKEN_UNIQUE_SYMBOL_MAX_LEN+1+78+1+1) // "VITE-000+space+78digits+1period+1\0"


typedef struct {
    libv_token_id_t tokenId;
    char suffix[TOKEN_UNIQUE_SYMBOL_MAX_LEN];
    uint8_t unitScale;
} TOKEN_INFO;


#define CONTRACT_ADDRESS_NAME_MAX_LEN 30

typedef struct {
    char addressName[CONTRACT_ADDRESS_NAME_MAX_LEN];
} CONTRACT_INFO;

#define CONTRACT_INFO_ARRAY_LEN 8
extern CONTRACT_INFO const CONTRACT_INFO_ARRAY[CONTRACT_INFO_ARRAY_LEN];

#define TOKEN_INFO_ARRAY_LEN 96
extern TOKEN_INFO const TOKEN_INFO_ARRAY[TOKEN_INFO_ARRAY_LEN];

#endif // LIBV_ROM_VARIABLES_H
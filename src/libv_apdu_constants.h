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

#ifndef LIBV_APDU_CONSTANTS_H

#define LIBV_APDU_CONSTANTS_H

#define LIBV_CLA 0xA1

#define LIBV_INS_GET_APP_CONF 0x01
#define LIBV_INS_GET_ADDRESS 0x02
#define LIBV_INS_SIGN_RESPONSE_BLOCK 0x03
#define LIBV_INS_CACHE_REQUEST_BLOCK_DATA 0x04
#define LIBV_INS_SIGN_REQUEST_BLOCK 0x05
#define LIBV_INS_GET_TOKEN_INFO 0x06
#define LIBV_INS_TEST_AMOUNT_TEXT 0x07

#define LIBV_SW_INCORRECT_LENGTH 0x6700
#define LIBV_SW_SECURITY_STATUS_NOT_SATISFIED 0x6982
#define LIBV_SW_CONDITIONS_OF_USE_NOT_SATISFIED 0x6985
#define LIBV_SW_INCORRECT_DATA 0x6A80
#define LIBV_SW_INVALID_SIGNATURE 0x6A81
#define LIBV_SW_PARENT_BLOCK_CACHE_MISS 0x6A82
#define LIBV_SW_ARRAY_OUT_OF_BOUNDS 0x6A83
#define LIBV_SW_INCORRECT_P1_P2 0x6B00
#define LIBV_SW_INS_NOT_SUPPORTED 0x6D00
#define LIBV_SW_CLA_NOT_SUPPORTED 0x6E00
#define LIBV_SW_TECHNICAL_PROBLEM 0x6F00
#define LIBV_SW_OK 0x9000
#define LIBV_SW_HALTED 0x6FAA
#define LIBV_SW_APP_HALTED LIBV_SW_CONDITIONS_OF_USE_NOT_SATISFIED

#define ISO_OFFSET_CLA 0x00
#define ISO_OFFSET_INS 0x01
#define ISO_OFFSET_P1 0x02
#define ISO_OFFSET_P2 0x03
#define ISO_OFFSET_LC 0x04
#define ISO_OFFSET_CDATA 0x05

#include "os.h"
#include "libv_secure_value.h"

#endif

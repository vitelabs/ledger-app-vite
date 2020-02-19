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

#ifndef LIBV_DEBUG_H
#define LIBV_DEBUG_H

#include "libv_internal.h"
#include "libv_apdu_constants.h"
#include "libv_bagl.h"

// #define LIBV_ENABLE_DEBUG

#define P2_DEBUG 0xde

uint16_t libv_apdu_test_output(libv_apdu_response_t *resp, uint8_t *buffer, uint64_t len);

#endif
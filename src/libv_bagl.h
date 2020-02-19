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

#ifndef LIBV_BAGL_H

#define LIBV_BAGL_H

#include "libv_app.h"

void libv_bagl_display_address_callback(bool confirmed);
void libv_bagl_confirm_sign_receive_block_callback(bool confirmed);
void libv_bagl_confirm_sign_send_block_callback(bool confirmed);

/** Apply current global state to UX. Returns true if UX was updated,
    false if the UX is already in the correct state and nothing was done. **/
bool libv_bagl_apply_state();

#endif // LIBV_BAGL_H

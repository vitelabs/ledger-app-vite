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

#ifndef LIBV_FS_H

#define LIBV_FS_H

#include <stdbool.h>

#include "os.h"
#include "libv_context.h"

typedef struct libv_storage_s {
    bool autoReceive;
} libv_storage_t;

// the global nvram memory variable
extern libv_storage_t const N_libv_real;
#define N_libv (*(volatile libv_storage_t *)PIC(&N_libv_real))

void libv_set_auto_receive(bool enabled);

#endif

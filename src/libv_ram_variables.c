/*******************************************************************************
*   Vite Wallet for Ledger Nano S
*   (c) 2020 Vite Labs
*   (c) 2016 Ledger
*   (c) 2018 Mart Roosmaa
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

#include "libv_ram_variables.h"
#include "os_io_seproxyhal.h"

// Bolos SDK variables
uint8_t G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

#if defined(TARGET_NANOX)
ux_state_t G_ux;
bolos_ux_params_t G_ux_params;
#else
ux_state_t ux;
#endif

uint8_t libv_async_buffer_D[MAX_ADPU_OUTPUT_SIZE + 2 /* status word */];
libv_context_t libv_context_D;

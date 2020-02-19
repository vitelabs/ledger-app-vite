#!/usr/bin/env python
#*******************************************************************************
#*   Vite Wallet for Ledger Nano S
#*   (c) 2020 Vite Labs
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#********************************************************************************
from ledgerblue.comm import getDongle
dongle = getDongle(True)

# Get config
dongle.exchange(bytes.fromhex("A1 01 00 00 00"))

# Get vite address
dongle.exchange(bytes.fromhex("A1 02 00 00 0D 03 8000002C 800a2c2a 80000000"))
# Get vite address and display
dongle.exchange(bytes.fromhex("A1 02 01 00 0D 03 8000002C 800a2c2a 80000000"))

# Sign receive block
dongle.exchange(bytes.fromhex("A1 03 00 00 5D 038000002C800a2c2a80000000 0000000000000000000000000000000000000000000000000000000000000000 0000000000000001 1e3004d74382a8635b836eb8a3e34ede7c00a7a1bff0c150974c1235287ad07a e0a56f09a7ec71b0"))

# Send 0.1 Vite
dongle.exchange(bytes.fromhex("A1 05 02 00 9C 038000002C800a2c2a80000000 0dc5bfb1f3fdba8cc339b506de2e987bb1744e41332140d6a1a69c4b41e79595 0000000000000002 48a6832a2850efb6e4549318835248a48078765c00 000000000000000000000000000000000000000000000000016345785d8a0000 5649544520544f4b454e 0000000000000000000000000000000000000000000000000000000000000000 daf9abd78d693034"))
# Send 100000000000000000 unknown token data: 30314041
dongle.exchange(bytes.fromhex("A1 05 02 00 A0 038000002C800a2c2a80000000 0dc5bfb1f3fdba8cc339b506de2e987bb1744e41332140d6a1a69c4b41e79595 0000000000000002 48a6832a2850efb6e4549318835248a48078765c00 000000000000000000000000000000000000000000000000016345785d8a0000 5649544520544f4b454f 0000000000000000000000000000000000000000000000000000000000000000 daf9abd78d693034 30314041"))

# Send 100000000000000000 unknown token data: 30314041
dongle.exchange(bytes.fromhex("A1 04 01 00 02 3031"))
dongle.exchange(bytes.fromhex("A1 04 02 00 02 4041"))
dongle.exchange(bytes.fromhex("A1 05 01 00 9C 038000002C800a2c2a80000000 0dc5bfb1f3fdba8cc339b506de2e987bb1744e41332140d6a1a69c4b41e79595 0000000000000002 48a6832a2850efb6e4549318835248a48078765c00 000000000000000000000000000000000000000000000000016345785d8a0000 5649544520544f4b454f 0000000000000000000000000000000000000000000000000000000000000000 daf9abd78d693034"))


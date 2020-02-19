# Vite application : Common Technical Specifications

## About

This specification describes the APDU messages interface to communicate with the Vite application.

## Wallet usage APDUs

### Get address

#### Description

This command returns the public key and the address for the given BIP 32 path.

#### Coding

**Command**

| *CLA* | *INS*  | *P1*                             | *P2* | *Lc* | *Le* |
|-------|--------|----------------------------------|------|------|------|
|   A1  |   02   |  00 : do not display the address |      |      |      |
|       |        |  01 : display the address        |      |      |      |

**Input data**

| *Description*                                     | *Length*  |
|---------------------------------------------------|-----------|
| Number of BIP 32 derivations to perform (max 10)  | 1         |
| First derivation index (big endian)               | 4         |
| ...                                               | 4         |
| Last derivation index (big endian)                | 4         |

**Output data**

| *Description*                                     | *Length*  |
|---------------------------------------------------|-----------|
| Public key                                        | 32        |
| Account address                                   | 55        |


### Sign response block

#### Description
This command returns hash and signature of a response block.

#### Coding

**Command**

| *CLA* | *INS*  | *P1* | *P2* | *Lc* | *Le* |
|-------|--------|------|------|------|------|
|   A1  |   03   |  00  |  00  |      |      |


**Input data**

| *Description*                                      | *Length*  |
|----------------------------------------------------|-----------|
| Number of BIP 32 derivations to perform (max 10)   | 1         |
| First derivation index (big endian)                | 4         |
| ...                                                | 4         |
| Last derivation index (big endian)                 | 4         |
| Previous Hash                                      | 32        |
| Height                                             | 8         |
| Send Block Hash                                    | 32        |
| Nonce                                              | 8         |

**Output data**

| *Description*                                      | *Length*  |
|----------------------------------------------------|-----------|
| Block hash                                         | 32        |
| Signature                                          | 64        |

### Cache request block data

#### Description
This command caches request block's data in memory, which can be cached seperately. The maximum cache volume each time is 224 Byte. Sign request block command will use part of cached data to calculate hash and signature.

#### Coding

**Command**

| *CLA* | *INS*  | *P1*          | *P2* | *Lc* | *Le* |
|-------|--------|---------------|------|------|------|
|   A1  |   04   |  *see below*  |  00  |      |      |

The ***P1*** value can compose of the following bitwise flags:

- `0x01` - First cached data
- `0x02` - Remained data

**Input data**

| *Description*                                      | *Length*  |
|----------------------------------------------------|-----------|
| data                                               | max 224   |

**Output data**

None

### Sign request block
This command returns hash and signature of request block. If data length is greater than 64 Byte, you need to call Cache request block data command to cache data before call this command.

#### Description

#### Coding

**Command**

| *CLA* | *INS*  | *P1*          | *P2* | *Lc* | *Le* |
|-------|--------|---------------|------|------|------|
|   A1  |   05   |  *see below*  |  00  |      |      |

The ***P1*** value can compose of the following bitwise flags:

- `0x01` - Cached data via Cache request block data command
- `0x02` - Cached data not by Cache request block data command
**Input data**

| *Description*                                      | *Length*  |
|----------------------------------------------------|-----------|
| Number of BIP 32 derivations to perform (max 10)   | 1         |
| First derivation index (big endian)                | 4         |
| ...                                                | 4         |
| Last derivation index (big endian)                 | 4         |
| Previous Hash                                      | 32        |
| Height                                             | 8         |
| To Address                                         | 21        |
| Amount                                             | 32        |
| Token ID                                           | 10        |
| fee                                                | 32        |
| Nonce                                              | 8         |
| data                                               | max 64    |

**Output data**

| *Description*                                      | *Length*  |
|----------------------------------------------------|-----------|
| Block hash                                         | 32        |
| Signature                                          | 64        |


## Test and utility APDUs

### Get app configuration

#### Description

This command returns the application configuration.

#### Coding

**Command**

| *CLA* | *INS*  | *P1*                 | *P2* | *Lc* | *Le* |
|-------|--------|----------------------|------|------|------|
|   A1  |   01   |  00                  |  00  |  00  |      |

**Input data**

*None*

**Output data**

| *Description*                                      | *Length*  |
|----------------------------------------------------|-----------|
| Major app version                                  | 1         |
| Minor app version                                  | 1         |
| Patch app version                                  | 1         |
| Builtin token count                                | 2         |


### Get builtin token info

#### Description

This command gets the built-in token info from application

#### Coding

**Command**

| *CLA* | *INS*  | *P1*                 | *P2* | *Lc* | *Le* |
|-------|--------|----------------------|------|------|------|
|   A1  |   06   |  00                  |  00  |      |      |

**Input data**

| *Description*                                      | *Length*  |
|----------------------------------------------------|-----------|
| Index (big endian)                                 | 2         |

**Output data**

| *Description*                                      | *Length*  |
|----------------------------------------------------|-----------|
| Token ID string                                    | 28        |
| Token decimals                                     | 1         |
| Token symbol and index                             | var       |

### Test display script of confirm amount

#### Description

This command returns formatted confirm amount and text information on the basis of input amount and token ID. This command merely uses for testing the correctness of built-in token info

#### Coding

**Command**

| *CLA* | *INS*  | *P1*                 | *P2* | *Lc* | *Le* |
|-------|--------|----------------------|------|------|------|
|   A1  |   07   |  00                  |  00  |      |      |

**Input data**

| *Description*                                      | *Length*  |
|----------------------------------------------------|-----------|
| Amount                                             | 32        |
| Token ID                                           | 10        |

**Output data**

| *Description*                                      | *Length*  |
|----------------------------------------------------|-----------|
| Text                                               | var       |

## Transport protocol

### General transport description

Ledger APDUs requests and responses are encapsulated using a flexible protocol allowing to fragment large payloads over different underlying transport mechanisms. 

The common transport header is defined as follows:

| *Description*                                                                     | *Length* |
|-----------------------------------------------------------------------------------|----------|
| Communication channel ID (big endian)                                             | 2        |
| Command tag                                                                       | 1        |
| Packet sequence index (big endian)                                                | 2        |
| Payload                                                                           | var      |

The Communication channel ID allows commands multiplexing over the same physical link. It is not used for the time being, and should be set to 0101 to avoid compatibility issues with implementations ignoring a leading 00 byte.

The Command tag describes the message content. Use TAG_APDU (0x05) for standard APDU payloads, or TAG_PING (0x02) for a simple link test.

The Packet sequence index describes the current sequence for fragmented payloads. The first fragment index is 0x00.

### APDU Command payload encoding

APDU Command payloads are encoded as follows:

| *Description*                                                                     | *Length* |
|-----------------------------------------------------------------------------------|----------|
| APDU length (big endian)                                                          | 2        |
| APDU CLA                                                                          | 1        |
| APDU INS                                                                          | 1        |
| APDU P1                                                                           | 1        |
| APDU P2                                                                           | 1        |
| APDU length                                                                       | 1        |
| Optional APDU data                                                                | var      |

APDU payload is encoded according to the APDU case 

| Case Number  | *Lc* | *Le* | Case description                                          |
|--------------|------|------|-----------------------------------------------------------|
|   1          |  0   |  0   | No data in either direction - L is set to 00              |
|   2          |  0   |  !0  | Input Data present, no Output Data - L is set to Lc       |
|   3          |  !0  |  0   | Output Data present, no Input Data - L is set to Le       |
|   4          |  !0  |  !0  | Both Input and Output Data are present - L is set to Lc   |

### APDU Response payload encoding

APDU Response payloads are encoded as follows:

| *Description*                                                                     | *Length* |
|-----------------------------------------------------------------------------------|----------|
| APDU response length (big endian)                                                 | 2        |
| APDU response data and Status Word                                                | var      |

### USB mapping

Messages are exchanged with the dongle over HID endpoints over interrupt transfers, with each chunk being 64 bytes long. The HID Report ID is ignored.

## Status words 

The following standard Status Words are returned for all APDUs - some specific Status Words can be used for specific commands and are mentioned in the command description.

**Status words**

|   *SW*   | *Description*                                                                 |
|----------|-------------------------------------------------------------------------------|
|   6700   | Incorrect length                                                              |
|   6982   | Security status not satisfied (dongle is locked or busy with another request) |
|   6985   | User declined the request                                                     |
|   6A80   | Invalid input data                                                            |
|   6A81   | Failed to verify the provided signature                                     |
|   6A82   | Parent block data cache-miss (cache parent before sign)                    |
|   6B00   | Incorrect parameter P1 or P2                                                  |
|   6Fxx   | Technical problem (Internal error, please report)                             |
|   9000   | Normal ending of the command                                                  |

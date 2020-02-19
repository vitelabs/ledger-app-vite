#ifndef ED25519_H
#define ED25519_H

#include <stddef.h>
#include "libv_types.h"

void ed25519_publickey(const libv_private_key_t privateKey,
                       libv_public_key_t publicKey);

void ed25519_sign(const uint8_t *m, size_t mlen,
                  const libv_private_key_t privateKey,
                  const libv_public_key_t publicKey,
                  libv_signature_t signature);

int ed25519_sign_open(const uint8_t *m, size_t mlen,
                      const libv_public_key_t publicKey,
                      const libv_signature_t signature);

#endif // ED25519_H

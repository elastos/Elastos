/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <cjose/cjose.h>
#include <jansson.h>

#include "ela_did.h"
#include "ela_jwt.h"
#include "crypto.h"
#include "HDkey.h"
#include "jws.h"
#include "diderror.h"

static cjose_jwk_t *get_jwk(JWS *jws)
{
    cjose_err err;
    DID *issuer = NULL;
    DIDDocument *doc = NULL;
    DIDURL *keyid;
    PublicKey *key;
    const char *keybase58;
    uint8_t binkey[PUBLICKEY_BYTES];
    KeySpec _spec, *spec;
    cjose_jwk_t *jwk = NULL;
    int rc = -1;

    assert(jws);
    assert(jws->header);
    assert(jws->claims);

    issuer = DID_FromString(JWS_GetIssuer(jws));
    if (!issuer)
        goto errorExit;

    doc = DID_Resolve(issuer, false);
    if (!doc)
        goto errorExit;

    keyid = DIDURL_FromString(JWS_GetKeyId(jws), NULL);
    if (!keyid)
        goto errorExit;

    key = DIDDocument_GetPublicKey(doc, keyid);
    if (!key)
        goto errorExit;

    keybase58 = PublicKey_GetPublicKeyBase58(key);
    if (!keybase58)
        goto errorExit;

    base58_decode(binkey, sizeof(binkey), keybase58);

    memset(&_spec, 0, sizeof(KeySpec));
    spec = KeySpec_Fill(&_spec, binkey, NULL);
    if (!spec) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Get key spec failed.");
        goto errorExit;
    }

    jwk = cjose_jwk_create_EC_spec((cjose_jwk_ec_keyspec*)spec, &err);
    if (!jwk) {
        DIDError_Set(DIDERR_JWT, "Create jwk failed.");
        goto errorExit;
    }

errorExit:
    if (issuer)
        DID_Destroy(issuer);
    if (doc)
        DIDDocument_Destroy(doc);

    return jwk;
}

JWS *JWTParser_Parse(const char *token)
{
    JWS *jws = NULL;
    cjose_err err;
    char *payload = NULL;
    size_t payload_len = 0;
    bool successed;
    cjose_jwk_t *jwk = NULL;
    DIDURL *signkey;

    if (!token || !*token) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    jws = (JWS *)calloc(1, sizeof(JWS));
    if (!jws) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Remalloc buffer for JWS failed.");
        return NULL;
    }

    //set jws
    jws->jws = cjose_jws_import(token, strlen(token), &err);
    if (!jws->jws) {
        DIDError_Set(DIDERR_JWT, "Import token to jws failed.");
        JWS_Destroy(jws);
        return NULL;
    }

    //get header
    jws->header = cjose_jws_get_protected(jws->jws);
    if (!jws->header) {
        DIDError_Set(DIDERR_JWT, "Get jwt header failed.");
        JWS_Destroy(jws);
        return NULL;
    }

    //set claims(payload)
    successed = cjose_jws_get_plaintext(jws->jws, (uint8_t**)&payload, &payload_len, &err);
    if (!successed) {
        DIDError_Set(DIDERR_JWT, "Get jwt body failed.");
        JWS_Destroy(jws);
        return NULL;
    }

    jws->claims = json_loadb(payload, payload_len, 0, NULL);;
    if (!jws->claims) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Load jwt body failed.");
        JWS_Destroy(jws);
        return NULL;
    }

    //get jwk, must put after getting header and claims.
    jwk = get_jwk(jws);
    if (!jwk) {
        JWS_Destroy(jws);
        return NULL;
    }

    successed = cjose_jws_verify(jws->jws, jwk, &err);
    cjose_jwk_release(jwk);
    if (!successed) {
        DIDError_Set(DIDERR_JWT, "Verify jws failed.");
        JWS_Destroy(jws);
        return NULL;
    }

    return jws;
}









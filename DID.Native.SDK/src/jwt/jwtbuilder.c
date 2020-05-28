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
#include <jansson.h>
#include <cjose/cjose.h>
#include <assert.h>

#include "ela_jwt.h"
#include "jwtbuilder.h"
#include "crypto.h"
#include "HDkey.h"
#include "claims.h"
#include "diderror.h"
#include "didstore.h"

static int init_jwtbuilder(JWTBuilder *builder)
{
    cjose_err err;
    char idstring[ELA_MAX_DID_LEN];

    assert(builder);

    builder->header = cjose_header_new(&err);
    if (!builder->header) {
        DIDError_Set(DIDERR_JWT, "Create jwt header failed.");
        return -1;
    }

    if (!cjose_header_set(builder->header, CJOSE_HDR_ALG, CJOSE_HDR_ALG_ES256, &err)) {
        DIDError_Set(DIDERR_JWT, "Set jwt algorithm failed.");
        return -1;
    }

    builder->claims = json_object();
    if (!builder->claims) {
        DIDError_Set(DIDERR_JWT, "Create claim object failed.");
        return -1;
    }

    if (!JWTBuilder_SetIssuer(builder, DID_ToString(&builder->issuer, idstring, sizeof(idstring)))) {
        DIDError_Set(DIDERR_JWT, "Set jwt issuer failed.");
        return -1;
    }

    return 0;
}

JWTBuilder *JWTBuilder_Create(DID *issuer)
{
    cjose_err err;

    if (!issuer) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!DIDMeta_AttachedStore(&issuer->meta)) {
        DIDError_Set(DIDERR_MALFORMED_DID, "Not attached with DID store.");
        return NULL;
    }

    JWTBuilder *builder = (JWTBuilder*)calloc(1, sizeof(JWTBuilder));
    if (!builder) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Remalloc buffer for JWTBuilder failed.");
        return NULL;
    }

    DID_Copy(&builder->issuer, issuer);
    builder->doc = DIDStore_LoadDID(issuer->meta.store, issuer);
    if (!builder->doc) {
        JWTBuilder_Destroy(builder);
        return NULL;
    }

    if (init_jwtbuilder(builder) == -1) {
        JWTBuilder_Destroy(builder);
        return NULL;
    }

    return builder;
}

void JWTBuilder_Destroy(JWTBuilder *builder)
{
    if (!builder)
        return;

    if (builder->header)
        cjose_header_release(builder->header);
    if (builder->jws)
        cjose_jws_release(builder->jws);
    if (builder->claims)
        json_decref(builder->claims);
    if (builder->doc)
        DIDDocument_Destroy(builder->doc);
}

bool JWTBuilder_SetHeader(JWTBuilder *builder, const char *attr, const char *value)
{
    cjose_err err;
    bool succussed;

    if (!builder || !attr || !*attr || !value) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (!builder->header) {
        DIDError_Set(DIDERR_JWT, "No header in jwt builder.");
        return false;
    }

    if (!strcmp(attr, CJOSE_HDR_ALG) || !strcmp(attr, CJOSE_HDR_KID)) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Cann't set algorithm or sign key!!!!");
        return false;
    }

    succussed = cjose_header_set(builder->header, attr, value, &err);
    if (!succussed)
        DIDError_Set(DIDERR_JWT, "Set header '%s' failed.", attr);

    return succussed;
}

bool JWTBuilder_SetClaim(JWTBuilder *builder, const char *key, const char *value)
{
    json_t *value_obj;
    int rc;
    bool succussed;

    if (!builder || !key || !*key || !value) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    value_obj = json_string(value);
    if (!value_obj) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Get value json failed.");
        return false;
    }

    rc = json_object_set_new(builder->claims, key, value_obj);
    succussed = (rc == -1) ? false : true;
    if (!succussed)
        DIDError_Set(DIDERR_JWT, "Set claim '%s' failed.", key);

    return succussed;
}

bool JWTBuilder_SetClaimWithJson(JWTBuilder *builder, const char *key, const char *json)
{
    json_t *json_obj;
    int rc;
    bool succussed;

    if (!builder || !key || !*key || !json || !*json) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    json_obj = json_loads(json, 0, NULL);
    if (!json_obj) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Load json object failed.");
        return false;
    }

    rc = json_object_set_new(builder->claims, key, json_obj);
    succussed = (rc == -1) ? false : true;
    if (!succussed)
        DIDError_Set(DIDERR_JWT, "Set json claim '%s' failed.", key);

    return succussed;
}

bool JWTBuilder_SetClaimWithIntegar(JWTBuilder *builder, const char *key, long value)
{
    json_t *value_obj;
    int rc;
    bool succussed;

    if (!builder || !key || !*key || !value) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    value_obj = json_integer(value);
    if (!value_obj) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Get value json failed.");
        return false;
    }

    rc = json_object_set_new(builder->claims, key, value_obj);
    succussed = (rc == -1) ? false : true;
    if (!succussed)
        DIDError_Set(DIDERR_JWT, "Set claim '%s' failed.", key);

    return succussed;
}

bool JWTBuilder_SetClaimWithBoolean(JWTBuilder *builder, const char *key, bool value)
{
    json_t *value_obj;
    int rc;
    bool succussed;

    if (!builder || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    value_obj = json_boolean(value);
    if (!value_obj) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Get value json failed.");
        return false;
    }

    rc = json_object_set_new(builder->claims, key, value_obj);
    succussed = (rc == -1) ? false : true;
    if (!succussed)
        DIDError_Set(DIDERR_JWT, "Set claim '%s' failed.", key);

    return succussed;
}

bool JWTBuilder_SetIssuer(JWTBuilder *builder, const char *issuer)
{
    return JWTBuilder_SetClaim(builder, ISSUER, issuer);
}

bool JWTBuilder_SetSubject(JWTBuilder *builder, const char *subject)
{
    return JWTBuilder_SetClaim(builder, SUBJECT, subject);
}

bool JWTBuilder_SetAudience(JWTBuilder *builder, const char *audience)
{
    return JWTBuilder_SetClaim(builder, AUDIENCE, audience);
}

bool JWTBuilder_SetExpiration(JWTBuilder *builder, time_t expire)
{
    return JWTBuilder_SetClaimWithIntegar(builder, EXPIRATION, expire);
}

bool JWTBuilder_SetNotBefore(JWTBuilder *builder, time_t nbf)
{
    return JWTBuilder_SetClaimWithIntegar(builder, NOT_BEFORE, nbf);
}

bool JWTBuilder_SetIssuedAt(JWTBuilder *builder, time_t iat)
{
    return JWTBuilder_SetClaimWithIntegar(builder, ISSUER_AT, iat);
}

bool JWTBuilder_SetId(JWTBuilder *builder, const char *jti)
{
    return JWTBuilder_SetClaim(builder, ID, jti);
}

int JWTBuilder_Sign(JWTBuilder *builder, DIDURL *keyid, const char *storepass)
{
    cjose_err err;
    cjose_jwk_t *jwk = NULL;
    PublicKey *pk;
    uint8_t pubkey[PUBLICKEY_BYTES];
    uint8_t privatekey[PRIVATEKEY_BYTES];
    char idstring[ELA_MAX_DID_LEN];
    const char *payload;
    KeySpec _keyspec, *keyspec;
    DID *issuer;

    if (!builder || !storepass || !*storepass)
        return -1;

    issuer = &builder->issuer;
    //get pk
    if (!keyid)
        keyid = DIDDocument_GetDefaultPublicKey(builder->doc);

    pk = DIDDocument_GetPublicKey(builder->doc, keyid);
    if (!pk) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Key no exist.");
        return -1;
    }
    base58_decode(pubkey, sizeof(pubkey), PublicKey_GetPublicKeyBase58(pk));

    //get sk
    if (!DIDStore_ContainsPrivateKey(issuer->meta.store, issuer, keyid))
        return -1;

    if (DIDStore_LoadPrivateKey(issuer->meta.store, storepass, issuer, keyid, privatekey) == -1)
        return -1;

    // create key spec
    keyspec = KeySpec_Fill(&_keyspec, pubkey, privatekey);
    memset(privatekey, 0, sizeof(privatekey));
    if (!keyspec)
        return -1;

    jwk = cjose_jwk_create_EC_spec((cjose_jwk_ec_keyspec*)keyspec, &err);
    if (!jwk) {
        DIDError_Set(DIDERR_JWT, "Create jwk failed.");
        return -1;
    }

    if (!cjose_header_set(builder->header, CJOSE_HDR_KID,
            DIDURL_ToString(keyid, idstring, sizeof(idstring), false), &err)) {
        DIDError_Set(DIDERR_JWT, "Set jwt sign key failed.");
        cjose_jwk_release(jwk);
        return -1;
    }

    payload = json_dumps(builder->claims, 0);
    if (!payload) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Get jwt body string failed.");
        cjose_jwk_release(jwk);
        return -1;
    }

    builder->jws = cjose_jws_sign(jwk, builder->header, (uint8_t*)payload, strlen(payload), &err);
    free((void*)payload);
    if (!builder->jws) {
        DIDError_Set(DIDERR_JWT, "Sign jwt body failed.");
        cjose_jwk_release(jwk);
        return -1;
    }

    return 0;
}

static const char *jws_export(JWTBuilder *builder)
{
    cjose_err err;
    bool exported;
    const char *payload = NULL;

    assert(builder);
    assert(builder->header);
    assert(builder->claims);

    exported = cjose_jws_export(builder->jws, &payload, &err);
    if (!exported) {
        DIDError_Set(DIDERR_JWT, "Export token failed.");
        return NULL;
    }

    return strdup(payload);
}

static const char *jwt_export(JWTBuilder *builder)
{
    const char *header = NULL, *claims = NULL;
    char *payload = NULL, *token = NULL;
    size_t len, token_len = 0;

    assert(builder);
    assert(builder->header);
    assert(builder->claims);

    //get header part in jwt
    header = json_dumps(builder->header, 0);
    if (!header) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Get jwt header string failed.");
        goto errorExit;
    }

    claims = json_dumps(builder->claims, 0);
    if (!claims) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Get jwt body string failed.");
        goto errorExit;
    }

    token = (char*)malloc(strlen(header) * 4 / 3 + strlen(claims) * 4 / 3 + 32 + 3);
    if (!token) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for token failed.");
        goto errorExit;
    }

    len = base64_url_encode(token, (const uint8_t *)header, strlen(header));
    free((char*)header);
    if (len <= 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Encode jwt header failed");
        goto errorExit;
    }
    token_len += len;
    token[token_len++] = '.';

    len = base64_url_encode(token + token_len, (const uint8_t *)claims, strlen(claims));
    free((char*)claims);
    if (len <= 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Encode jwt body failed");
        goto errorExit;
    }
    token_len += len;
    token[token_len++] = '.';
    token[token_len] = 0;

    return token;

errorExit:
    if (header)
        free((char*)header);
    if (claims)
        free((char*)claims);
    if (token)
        free((char*)token);

    return NULL;
}

const char *JWTBuilder_Compact(JWTBuilder *builder)
{
    if (!builder) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (!builder->header || !builder->claims) {
        DIDError_Set(DIDERR_INVALID_ARGS, "No header or claims in jwt builder.");
        return NULL;
    }

    if (!builder->jws)
        return jwt_export(builder);

    return jws_export(builder);
}

int JWTBuilder_Reset(JWTBuilder *builder)
{
    if (!builder) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (builder->header) {
        cjose_header_release(builder->header);
        builder->header = NULL;
    }
    if (builder->claims) {
        json_decref(builder->claims);
        builder->claims = NULL;
    }
    if (builder->jws) {
        cjose_jws_release(builder->jws);
        builder->jws = NULL;
    }

    return init_jwtbuilder(builder);
}

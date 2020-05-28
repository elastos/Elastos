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
#include <time.h>

#include "ela_jwt.h"
#include "jws.h"
#include "claims.h"
#include "diderror.h"

void JWS_Destroy(JWS *jws)
{
    if (!jws)
        return;

    if (jws->header)
        json_decref(jws->header);
    if (jws->claims)
        json_decref(jws->claims);
}

const char *JWS_GetHeader(JWS *jws, const char *attr)
{
    cjose_err err;
    const char *data;

    if (!jws || !attr) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    data = cjose_header_get(jws->header, attr, &err);
    if (!data) {
        DIDError_Set(DIDERR_JWT, "Get header '%s' failed.", attr);
        return NULL;
    }

    return data;
}

const char *JWS_GetAlgorithm(JWS *jws)
{
    return JWS_GetHeader(jws, CJOSE_HDR_ALG);
}

const char *JWS_GetKeyId(JWS *jws)
{
    return JWS_GetHeader(jws, CJOSE_HDR_KID);
}

const char *JWS_GetClaim(JWS *jws, const char *key)
{
    json_t *value;
    const char *data;

    if (!jws || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    value = json_object_get(jws->claims, key);
    if (!value) {
        DIDError_Set(DIDERR_JWT, "No claim: %s.", key);
        return NULL;
    }

    if (!json_is_string(value)) {
        DIDError_Set(DIDERR_JWT, "Claim '%s' is not string.", key);
        return NULL;
    }

    data = json_string_value(value);
    if (!data) {
        DIDError_Set(DIDERR_JWT, "Get claim '%s' string failed.", key);
        return NULL;
    }

    return data;
}

const char *JWS_GetClaimAsJson(JWS *jws, const char *key)
{
    json_t *value;
    const char *data;

    if (!jws || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    value = json_object_get(jws->claims, key);
    if (!value) {
        DIDError_Set(DIDERR_JWT, "No claim: %s.", key);
        return NULL;
    }

    if (json_is_object(value)) {
        data = json_dumps(value, 0);
        if (!data)
            DIDError_Set(DIDERR_JWT, "Get claim '%s' from json object failed.", key);

        return data;
    }

    if (json_is_array(value)) {
        data = json_dumps(value, 0);
        if (!data)
            DIDError_Set(DIDERR_JWT, "Get claim '%s' from json array failed.", key);

        return data;
    }

    DIDError_Set(DIDERR_UNSUPPOTED, "Unsupport this claim type.");
    return NULL;
}

long JWS_GetClaimAsInteger(JWS *jws, const char *key)
{
    json_t *value;
    long data;

    if (!jws || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return 0;
    }

    value = json_object_get(jws->claims, key);
    if (!value) {
        DIDError_Set(DIDERR_JWT, "No claim: %s.", key);
        return 0;
    }
    if (!json_is_integer(value)) {
        DIDError_Set(DIDERR_JWT, "Claim '%s' is not integar.", key);
        return 0;
    }

    return json_integer_value(value);
}

bool JWS_GetClaimAsBoolean(JWS *jws, const char *key)
{
    json_t *value;

    if (!jws || !key || !*key) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    value = json_object_get(jws->claims, key);
    if (!value) {
        DIDError_Set(DIDERR_JWT, "No claim: %s.", key);
        return false;
    }
    if (!json_is_boolean(value)) {
        DIDError_Set(DIDERR_JWT, "Claim '%s' is not boolean.", key);
        return false;
    }

    return json_boolean_value(value);
}

const char *JWS_GetIssuer(JWS *jws)
{
    return JWS_GetClaim(jws, ISSUER);
}

const char *JWS_GetSubject(JWS *jws)
{
    return JWS_GetClaim(jws, SUBJECT);
}

const char *JWS_GetAudience(JWS *jws)
{
    return JWS_GetClaim(jws, AUDIENCE);
}

const char *JWS_GetId(JWS *jws)
{
    return JWS_GetClaim(jws, ID);
}

time_t JWS_GetExpiration(JWS *jws)
{
    return JWS_GetClaimAsInteger(jws, EXPIRATION);
}

time_t JWS_GetNotBefore(JWS *jws)
{
    return JWS_GetClaimAsInteger(jws, NOT_BEFORE);
}

time_t JWS_GetIssuedAt(JWS *jws)
{
    return JWS_GetClaimAsInteger(jws, ISSUER_AT);
}

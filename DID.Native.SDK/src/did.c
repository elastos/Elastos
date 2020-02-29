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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "ela_did.h"
#include "did.h"
#include "diddocument.h"
#include "didstore.h"
#include "credential.h"
#include "didmeta.h"

static const char did_scheme[] = "did";
static const char did_method[] = "elastos";
static const char elastos_did_prefix[] = "did:elastos:";

// idstring has three informats:
// 1. did:elastos:xxxxxxx
// 2. did:elastos:xxxxxxx#xxxxx
// 3. #xxxxxxx
static int parse_id_string(char *id, char *fragment, const char *idstring, DID *ref)
{
    const char *s, *e;
    size_t len;

    assert(id);
    assert(idstring && *idstring);

    // Fragment only, need ref DID object
    if (*idstring == '#') {
        if (!fragment || !ref)
            return -1;

        len = strlen(++idstring);
        if (len == 0 || len >= MAX_FRAGMENT)
            return -1;

        strcpy(id, ref->idstring);
        strcpy(fragment, idstring);
        return 0;
    }

    if (strncmp(idstring, elastos_did_prefix, sizeof(elastos_did_prefix) - 1) != 0)
        return -1;

    s = idstring + sizeof(elastos_did_prefix) - 1;
    for (e = s; *e != '#' && *e != '?' && *e != '/' && *e != '\x0'; e++);
    len = e - s;
    if (len >= MAX_ID_SPECIFIC_STRING || len == 0)
        return -1;

    strncpy(id, s, len);
    id[len] = 0;

    if (!fragment)
        return 0;

    for (; *e != '#' && *e != '\x0'; e++);
    if (*e != '#')
        return -1;

    len = strlen(++e);
    if (len == 0 || len >= MAX_FRAGMENT)
        return -1;

    strcpy(fragment, e);
    return 0;
}

int parse_did(DID *did, const char *idstring)
{
    return parse_id_string(did->idstring, NULL, idstring, NULL);
}

int parse_didurl(DIDURL *id, const char *idstring, DID *ref)
{
    return parse_id_string(id->did.idstring, id->fragment, idstring, ref);
}

int init_didurl(DIDURL *id, DID *did, const char *fragment)
{
    if (!id || !did || !fragment || !fragment || strlen(fragment) >= sizeof(id->fragment))
        return -1;

    DID_Copy(&id->did, did);
    strcpy(id->fragment, fragment);
    return 0;
}

int init_did(DID *did, const char *idstring)
{
    if (!did || !idstring || !*idstring
            || strlen(idstring) >= sizeof(did->idstring))
        return -1;

    strcpy(did->idstring, idstring);
    return 0;
}

DID *DID_FromString(const char *idstring)
{
    DID *did;

    if (!idstring || !*idstring)
        return NULL;

    did = (DID *)calloc(1, sizeof(DID));
    if (!did)
        return NULL;

    if (parse_did(did, idstring) < 0) {
        free(did);
        return NULL;
    }

    return did;
}

DID *DID_New(const char *method_specific_string)
{
    DID *did;

    if (!method_specific_string || !*method_specific_string ||
        strlen(method_specific_string) >= MAX_ID_SPECIFIC_STRING)
        return NULL;

    did = (DID *)calloc(1, sizeof(DID));
    if (!did)
        return NULL;

    strcpy(did->idstring, method_specific_string);

    return did;
}

const char *DID_GetMethod(DID *did)
{
    if (!did)
        return NULL;

    return did_method;
}

const char *DID_GetMethodSpecificId(DID *did)
{
    if (!did)
        return NULL;

    return (const char *)did->idstring;
}

char *DID_ToString(DID *did, char *idstring, size_t len)
{
    if (!did || !idstring)
        return NULL;

    if (strlen(did->idstring) + strlen(elastos_did_prefix) >= len)
        return NULL;

    strcpy(idstring, elastos_did_prefix);
    strcat(idstring, did->idstring);

    return idstring;
}

DID *DID_Copy(DID *dest, DID *src)
{
    if (!dest || !src)
        return NULL;

    strcpy(dest->idstring, src->idstring);

    if (DIDMeta_Copy(&dest->meta, &src->meta) == -1)
        return NULL;

    return dest;
}

bool DID_Equals(DID *did1, DID *did2)
{
    if (!did1 || !did2)
        return false;

    return strcmp(did1->idstring, did2->idstring) == 0;
}

int DID_Compare(DID *did1, DID *did2)
{
    if (!did1 || !did2)
        return -1;

    return strcmp(did1->idstring, did2->idstring);
}

void DID_Destroy(DID *did)
{
    if (!did)
        return;

    free(did);
}

DIDURL *DIDURL_FromString(const char *idstring, DID *ref)
{
    DIDURL *id;

    if (!idstring || !*idstring)
        return NULL;

    id = (DIDURL *)calloc(1, sizeof(DIDURL));
    if (!id)
        return NULL;

    if (parse_didurl(id, idstring, ref) < 0) {
        free(id);
        return NULL;
    }

    return id;
}

DIDURL *DIDURL_New(const char *method_specific_string, const char *fragment)
{
    DIDURL *id;

    if (!method_specific_string || !*method_specific_string ||
        !fragment || !*fragment)
        return NULL;

    if (strlen(method_specific_string) >= MAX_ID_SPECIFIC_STRING ||
        strlen(fragment) >= MAX_FRAGMENT)
        return NULL;

    id = (DIDURL *)calloc(1, sizeof(DIDURL));
    if (!id)
        return NULL;

    strcpy(id->did.idstring, method_specific_string);
    strcpy(id->fragment, fragment);

    return id;
}

DID_API DIDURL *DIDURL_NewByDid(DID *did, const char *fragment)
{
    DIDURL *id;

    if (!did || !fragment || !*fragment ||
            strlen(fragment) >= MAX_FRAGMENT)
        return NULL;

    id = (DIDURL*)calloc(1, sizeof(DIDURL));
    if (!id)
        return NULL;

    if (!DID_Copy(&id->did, did)) {
        free(id);
        return NULL;
    }

    strcpy(id->fragment, fragment);
    return id;
}

DID *DIDURL_GetDid(DIDURL *id)
{
    if (!id)
        return NULL;

    return &(id->did);
}

const char *DIDURL_GetFragment(DIDURL *id)
{
    if (!id)
        return NULL;

    return (const char*)id->fragment;
}

char *DIDURL_ToString(DIDURL *id, char *idstring, size_t len, bool compact)
{
    size_t expect_len = 0;
    int size;

    if (!id || !idstring)
        return NULL;

    expect_len += strlen(id->fragment) + 1;         /* #xxxx */
    expect_len += compact ? 0 : strlen(elastos_did_prefix) + strlen(id->did.idstring);

    if (expect_len >= len)
        return NULL;

    if (compact) {
        size = snprintf(idstring, len, "#%s", id->fragment);
        if (size < 0 || size > len)
            return NULL;
    } else {
        size = snprintf(idstring, len, "%s%s#%s", elastos_did_prefix,
            id->did.idstring, id->fragment);
        if (size < 0 || size > len)
            return NULL;
    }

    return idstring;
}

bool DIDURL_Equals(DIDURL *id1, DIDURL *id2)
{
    if (!id1 || !id2)
        return false;

    return (strcmp(id1->did.idstring, id2->did.idstring) == 0 &&
            strcmp(id1->fragment, id2->fragment) == 0);
}

int DIDURL_Compare(DIDURL *id1, DIDURL *id2)
{
    char _idstring1[ELA_MAX_DIDURL_LEN], _idstring2[ELA_MAX_DIDURL_LEN];
    char *idstring1, *idstring2;

    if (!id1 || !id2)
        return -1;

    idstring1 = DIDURL_ToString(id1, _idstring1, ELA_MAX_DIDURL_LEN, false);
    idstring2 = DIDURL_ToString(id2, _idstring2, ELA_MAX_DIDURL_LEN, false);
    if (!idstring1 || !idstring2)
        return -1;

    return strcmp(idstring1, idstring2);
}

DIDURL *DIDURL_Copy(DIDURL *dest, DIDURL *src)
{
    if (!dest || !src )
        return NULL;

    strcpy(dest->did.idstring, src->did.idstring);
    strcpy(dest->fragment, src->fragment);
    CredentialMeta_Copy(&dest->meta, &src->meta);

    return dest;
}

void DIDURL_Destroy(DIDURL *id)
{
    if (!id)
        return;

    free(id);
    id = NULL;
}

DIDDocument *DID_Resolve(DID *did)
{
    if (!did)
        return NULL;

    return DIDBackend_Resolve(did);
}

int DID_SetAlias(DID *did, const char *alias)
{
    if (!did)
        return -1;

    if (DIDMeta_SetAlias(&did->meta, alias) == -1)
        return -1;

    if (DIDMeta_AttachedStore(&did->meta))
        didstore_storedidmeta(did->meta.store, &did->meta, did);

    return 0;
}

const char *DID_GetAlias(DID *did)
{
    if (!did)
        return NULL;

    return DIDMeta_GetAlias(&did->meta);
}

const char *DID_GetTxid(DID *did)
{
    if (!did)
        return NULL;

    return DIDMeta_GetTxid(&did->meta);
}

bool DID_GetDeactived(DID *did)
{
    if (!did)
        return false;

    return DIDMeta_GetDeactived(&did->meta);
}

time_t DID_GetLastTransactionTimestamp(DID *did)
{
    if (!did)
        return 0;

    return DIDMeta_GetTimestamp(&did->meta);
}

//for Credential
int DIDURL_SetAlias(DIDURL *id, const char *alias)
{
    if (!id)
        return -1;

    if (CredentialMeta_SetAlias(&id->meta, alias) == -1)
        return -1;

    if (CredentialMeta_AttachedStore(&id->meta))
        didstore_storecredmeta(id->meta.store, &id->meta, id);

    return 0;
}

const char *DIDURL_GetAlias(DIDURL *id)
{
    if (!id)
        return NULL;

    return CredentialMeta_GetAlias(&id->meta);
}

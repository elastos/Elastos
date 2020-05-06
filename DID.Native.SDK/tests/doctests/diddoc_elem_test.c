#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <limits.h>

#include <CUnit/Basic.h>
#include "ela_did.h"
#include "loader.h"
#include "constant.h"
#include "diddocument.h"
#include "HDkey.h"
#include "credential.h"

static DIDDocument *doc;
static DID *did;
static DIDStore *store;

static void test_diddoc_get_publickey(void)
{
    PublicKey *pks[4];
    PublicKey *pk;
    DIDURL *id, *defaultkey, *primaryid;
    ssize_t size;
    int rc;
    bool isEquals;

    CU_ASSERT_EQUAL(DIDDocument_GetPublicKeyCount(doc), 4);

    size = DIDDocument_GetPublicKeys(doc, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 4);

    for (int i = 0; i < size; i++) {
        pk = pks[i];
        id = PublicKey_GetId(pk);

        isEquals = DID_Equals(did, &(id->did));
        CU_ASSERT_TRUE(isEquals);
        CU_ASSERT_STRING_EQUAL(default_type, PublicKey_GetType(pk));

        isEquals = DID_Equals(did, PublicKey_GetController(pk));
        if (!strcmp(id->fragment, "recovery")) {
            CU_ASSERT_FALSE(isEquals);
        } else {
            CU_ASSERT_TRUE(isEquals);
        }

        CU_ASSERT_TRUE(!strcmp(id->fragment, "primary") ||
                !strcmp(id->fragment, "key2") || !strcmp(id->fragment, "key3") ||
                !strcmp(id->fragment, "recovery"));
    }

    //PublicKey getter.
    defaultkey = DIDDocument_GetDefaultPublicKey(doc);
    CU_ASSERT_PTR_NOT_NULL(defaultkey);

    primaryid = DIDURL_NewByDid(did, "primary");
    CU_ASSERT_PTR_NOT_NULL(primaryid);
    pk = DIDDocument_GetPublicKey(doc, primaryid);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(primaryid, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    isEquals = DIDURL_Equals(primaryid, defaultkey);
    CU_ASSERT_TRUE(isEquals);

    id = DIDURL_NewByDid(did, "key2");
    CU_ASSERT_PTR_NOT_NULL(id);
    pk = DIDDocument_GetPublicKey(doc, id);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id);

    //Key not exist, should fail.
    id = DIDURL_NewByDid(did, "notExist");
    CU_ASSERT_PTR_NOT_NULL(id);
    pk = DIDDocument_GetPublicKey(doc, id);
    CU_ASSERT_PTR_NULL(pk);
    DIDURL_Destroy(id);

    // Selector
    size = DIDDocument_SelectPublicKeys(doc, default_type, defaultkey, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(PublicKey_GetId(pks[0]), primaryid);
    CU_ASSERT_TRUE(isEquals);

    size = DIDDocument_SelectPublicKeys(doc, NULL, defaultkey, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(PublicKey_GetId(pks[0]), primaryid);
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(primaryid);

    size = DIDDocument_SelectPublicKeys(doc, default_type, NULL, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 4);

    id = DIDURL_NewByDid(did, "key2");
    CU_ASSERT_PTR_NOT_NULL(id);
    size = DIDDocument_SelectPublicKeys(doc, default_type, id, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(PublicKey_GetId(pks[0]), id);
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(did, "key3");
    CU_ASSERT_PTR_NOT_NULL(id);
    size = DIDDocument_SelectPublicKeys(doc, NULL, id, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(PublicKey_GetId(pks[0]), id);
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id);
}

static void test_diddoc_add_publickey(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    const char *keybase;
    bool isEquals;
    int rc;

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    // Add 2 public keys
    DIDURL *id1 = DIDURL_NewByDid(did, "test1");
    CU_ASSERT_PTR_NOT_NULL(id1);
    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    rc = DIDDocumentBuilder_AddPublicKey(builder, id1, did, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *id2 = DIDURL_NewByDid(did, "test2");
    CU_ASSERT_PTR_NOT_NULL(id2);
    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    rc = DIDDocumentBuilder_AddPublicKey(builder, id2, did, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    // Check existence
    PublicKey *pk = DIDDocument_GetPublicKey(sealeddoc, id1);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id1, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id1);

    pk = DIDDocument_GetPublicKey(sealeddoc, id2);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id2, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id2);

    // Check the final count.
    CU_ASSERT_EQUAL(6, DIDDocument_GetPublicKeyCount(sealeddoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(sealeddoc));
    CU_ASSERT_EQUAL(1, DIDDocument_GetAuthorizationCount(sealeddoc));

    DIDDocument_Destroy(sealeddoc);
}

static void test_diddoc_remove_publickey(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    DIDURL *recoveryid, *keyid;
    int rc;

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    // recovery used by authorization, should failed.
    recoveryid = DIDURL_NewByDid(did, "recovery");
    CU_ASSERT_PTR_NOT_NULL(recoveryid);
    rc = DIDDocumentBuilder_RemovePublicKey(builder, recoveryid, false);
    CU_ASSERT_EQUAL(rc, -1);

    rc = DIDDocumentBuilder_RemovePublicKey(builder, recoveryid, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    keyid = DIDURL_NewByDid(did, "notExistKey");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_RemovePublicKey(builder, keyid, true);
    CU_ASSERT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    keyid = DIDURL_NewByDid(did, "key2");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_RemovePublicKey(builder, keyid, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = DIDDocumentBuilder_RemovePublicKey(builder,
            DIDDocument_GetDefaultPublicKey(doc), true);
    CU_ASSERT_EQUAL(rc, -1);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    // Check existence
    PublicKey *pk = DIDDocument_GetPublicKey(sealeddoc, recoveryid);
    CU_ASSERT_PTR_NULL(pk);
    DIDURL_Destroy(recoveryid);

    pk = DIDDocument_GetPublicKey(sealeddoc, keyid);
    CU_ASSERT_PTR_NULL(pk);
    DIDURL_Destroy(keyid);

    // Check the final count.
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(sealeddoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(sealeddoc));
    CU_ASSERT_EQUAL(0, DIDDocument_GetAuthorizationCount(sealeddoc));

    DIDDocument_Destroy(sealeddoc);
}

static void test_diddoc_get_authentication_key(void)
{
    PublicKey *pks[3];
    ssize_t size;
    PublicKey *pk;
    DIDURL *keyid, *id;
    bool isEquals;

    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(doc));

    size = DIDDocument_GetAuthenticationKeys(doc, pks, sizeof(pks));
    CU_ASSERT_EQUAL(3, size);

    for (int i = 0; i < size; i++) {
        pk = pks[i];
        id = PublicKey_GetId(pk);

        isEquals = DID_Equals(did, &id->did);
        CU_ASSERT_TRUE(isEquals);
        CU_ASSERT_STRING_EQUAL(default_type, PublicKey_GetType(pk));

        isEquals = DID_Equals(did, PublicKey_GetController(pk));
        CU_ASSERT_TRUE(isEquals);

        CU_ASSERT_TRUE(!strcmp(id->fragment, "primary") ||
                !strcmp(id->fragment, "key2") || !strcmp(id->fragment, "key3"));
    }

    // AuthenticationKey getter
    id = DIDURL_NewByDid(did, "primary");
    CU_ASSERT_PTR_NOT_NULL(id);
    pk = DIDDocument_GetAuthenticationKey(doc, id);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id);

    keyid = DIDURL_NewByDid(did, "key3");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    pk = DIDDocument_GetAuthenticationKey(doc, keyid);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(keyid, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);

    //Key not exist, should fail.
    id = DIDURL_NewByDid(did, "notExist");
    CU_ASSERT_PTR_NOT_NULL(id);
    pk = DIDDocument_GetAuthenticationKey(doc, id);
    CU_ASSERT_PTR_NULL(pk);
    DIDURL_Destroy(id);

    // Selector
    size = DIDDocument_SelectAuthenticationKeys(doc, default_type, keyid, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(PublicKey_GetId(pks[0]), keyid);
    CU_ASSERT_TRUE(isEquals);

    size = DIDDocument_SelectAuthenticationKeys(doc, NULL, keyid, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(PublicKey_GetId(pks[0]), keyid);
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(keyid);

    size = DIDDocument_SelectAuthenticationKeys(doc, default_type, NULL, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 3);

    id = DIDURL_NewByDid(did, "key2");
    CU_ASSERT_PTR_NOT_NULL(id);
    size = DIDDocument_SelectAuthenticationKeys(doc, default_type, id, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(PublicKey_GetId(pks[0]), id);
    CU_ASSERT_TRUE(isEquals);

    size = DIDDocument_SelectAuthenticationKeys(doc, NULL, id, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(PublicKey_GetId(pks[0]), id);
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id);
}

static void test_diddoc_add_authentication_key(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    const char *keybase;
    bool isEquals;
    int rc;

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    // Add 2 public keys
    DIDURL *id1 = DIDURL_NewByDid(did, "test1");
    CU_ASSERT_PTR_NOT_NULL(id1);
    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    rc = DIDDocumentBuilder_AddPublicKey(builder, id1, did, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, id1, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *id2 = DIDURL_NewByDid(did, "test2");
    CU_ASSERT_PTR_NOT_NULL(id2);
    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    rc = DIDDocumentBuilder_AddPublicKey(builder, id2, did, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, id2, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Add new keys
    DIDURL *id3 = DIDURL_NewByDid(did, "test3");
    CU_ASSERT_PTR_NOT_NULL(id3);
    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, id3, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *id4 = DIDURL_NewByDid(did, "test4");
    CU_ASSERT_PTR_NOT_NULL(id4);
    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, id4, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Try to add a non existing key, should fail.
    DIDURL *id = DIDURL_NewByDid(did, "notExistKey");
    CU_ASSERT_PTR_NOT_NULL(id);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, id, NULL);
    CU_ASSERT_EQUAL(rc, -1);
    DIDURL_Destroy(id);

    // Try to add a key not owned by self, should fail.
    id = DIDURL_NewByDid(did, "recovery");
    CU_ASSERT_PTR_NOT_NULL(id);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, id, NULL);
    CU_ASSERT_EQUAL(rc, -1);
    DIDURL_Destroy(id);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    // Check existence
    PublicKey *pk = DIDDocument_GetPublicKey(sealeddoc, id1);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id1, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id1);

    pk = DIDDocument_GetPublicKey(sealeddoc, id2);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id2, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id2);

    pk = DIDDocument_GetPublicKey(sealeddoc, id3);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id3, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id3);

    pk = DIDDocument_GetPublicKey(sealeddoc, id4);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id4, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id4);

    // Check the final count.
    CU_ASSERT_EQUAL(8, DIDDocument_GetPublicKeyCount(sealeddoc));
    CU_ASSERT_EQUAL(7, DIDDocument_GetAuthenticationCount(sealeddoc));
    CU_ASSERT_EQUAL(1, DIDDocument_GetAuthorizationCount(sealeddoc));

    DIDDocument_Destroy(sealeddoc);
}

static void test_diddoc_remove_authentication_key(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    const char *keybase;
    int rc;

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    // Add 2 public keys
    DIDURL *id1 = DIDURL_NewByDid(did, "test1");
    CU_ASSERT_PTR_NOT_NULL(id1);
    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, id1, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *id2 = DIDURL_NewByDid(did, "test2");
    CU_ASSERT_PTR_NOT_NULL(id2);
    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, id2, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Remote keys
    rc = DIDDocumentBuilder_RemoveAuthenticationKey(builder, id1);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = DIDDocumentBuilder_RemoveAuthenticationKey(builder, id2);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *id3 = DIDURL_NewByDid(did, "key2");
    CU_ASSERT_PTR_NOT_NULL(id3);
    rc = DIDDocumentBuilder_RemoveAuthenticationKey(builder, id3);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Key not exist, should fail.
    DIDURL *id = DIDURL_NewByDid(did, "notExistKey");
    CU_ASSERT_PTR_NOT_NULL(id);
    rc = DIDDocumentBuilder_RemoveAuthenticationKey(builder, id);
    CU_ASSERT_EQUAL(rc, -1);
    DIDURL_Destroy(id);

    // Default publickey, can not remove, should fail.
    rc = DIDDocumentBuilder_RemoveAuthenticationKey(builder,
            DIDDocument_GetDefaultPublicKey(doc));
    CU_ASSERT_EQUAL(rc, -1);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    PublicKey *pk = DIDDocument_GetAuthenticationKey(sealeddoc, id1);
    CU_ASSERT_PTR_NULL(pk);
    DIDURL_Destroy(id1);

    pk = DIDDocument_GetAuthenticationKey(sealeddoc, id2);
    CU_ASSERT_PTR_NULL(pk);
    DIDURL_Destroy(id2);

    pk = DIDDocument_GetAuthenticationKey(sealeddoc, id3);
    CU_ASSERT_PTR_NULL(pk);
    DIDURL_Destroy(id3);

    // Check the final count.
    CU_ASSERT_EQUAL(6, DIDDocument_GetPublicKeyCount(sealeddoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(sealeddoc));
    CU_ASSERT_EQUAL(1, DIDDocument_GetAuthorizationCount(sealeddoc));

    DIDDocument_Destroy(sealeddoc);
}

static void test_diddoc_get_authorization_key(void)
{
    PublicKey *pks[1];
    ssize_t size;
    PublicKey *pk;
    DIDURL *keyid, *id;
    bool isEquals;

    CU_ASSERT_EQUAL(1, DIDDocument_GetAuthorizationCount(doc));

    size = DIDDocument_GetAuthorizationKeys(doc, pks, sizeof(pks));
    CU_ASSERT_EQUAL(1, size);

    for (int i = 0; i < size; i++) {
        pk = pks[i];
        id = PublicKey_GetId(pk);

        isEquals = DID_Equals(did, &id->did);
        CU_ASSERT_TRUE(isEquals);
        CU_ASSERT_STRING_EQUAL(default_type, PublicKey_GetType(pk));

        isEquals = DID_Equals(did, PublicKey_GetController(pk));
        CU_ASSERT_FALSE(isEquals);

        CU_ASSERT_TRUE(!strcmp(id->fragment, "recovery"));
    }

    // AuthorizationKey getter
    keyid = DIDURL_NewByDid(did, "recovery");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    pk = DIDDocument_GetAuthorizationKey(doc, keyid);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(keyid, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);

    //Key not exist, should fail.
    id = DIDURL_NewByDid(did, "notExist");
    CU_ASSERT_PTR_NOT_NULL(id);
    pk = DIDDocument_GetAuthorizationKey(doc, id);
    CU_ASSERT_PTR_NULL(pk);
    DIDURL_Destroy(id);

    // Selector
    size = DIDDocument_SelectAuthorizationKeys(doc, default_type, keyid, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(PublicKey_GetId(pks[0]), keyid);
    CU_ASSERT_TRUE(isEquals);

    size = DIDDocument_SelectAuthorizationKeys(doc, NULL, keyid, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(PublicKey_GetId(pks[0]), keyid);
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(keyid);

    size = DIDDocument_SelectAuthorizationKeys(doc, default_type, NULL, pks, sizeof(pks));
    CU_ASSERT_EQUAL(size, 1);
}

static void test_diddoc_add_authorization_key(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    DerivedKey _dkey, *dkey;
    const char *keybase, *idstring;
    DID controller;
    bool isEquals;
    int rc;

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    // Add 2 public keys
    DIDURL *id1 = DIDURL_NewByDid(did, "test1");
    CU_ASSERT_PTR_NOT_NULL(id1);
    dkey = Generater_KeyPair(&_dkey);
    keybase = DerivedKey_GetPublicKeyBase58(dkey, publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    idstring = DerivedKey_GetAddress(dkey);
    CU_ASSERT_PTR_NOT_NULL(idstring);
    strncpy(controller.idstring, idstring, sizeof(controller.idstring));
    rc = DIDDocumentBuilder_AddPublicKey(builder, id1, &controller, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDDocumentBuilder_AddAuthorizationKey(builder, id1, &controller, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *id2 = DIDURL_NewByDid(did, "test2");
    CU_ASSERT_PTR_NOT_NULL(id2);
    dkey = Generater_KeyPair(&_dkey);
    keybase = DerivedKey_GetPublicKeyBase58(dkey, publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    idstring = DerivedKey_GetAddress(dkey);
    CU_ASSERT_PTR_NOT_NULL(idstring);
    strncpy(controller.idstring, idstring, sizeof(controller.idstring));
    rc = DIDDocumentBuilder_AddPublicKey(builder, id2, &controller, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDDocumentBuilder_AddAuthorizationKey(builder, id2, NULL, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Add new keys
    DIDURL *id3 = DIDURL_NewByDid(did, "test3");
    CU_ASSERT_PTR_NOT_NULL(id3);
    dkey = Generater_KeyPair(&_dkey);
    keybase = DerivedKey_GetPublicKeyBase58(dkey, publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    idstring = DerivedKey_GetAddress(dkey);
    CU_ASSERT_PTR_NOT_NULL(idstring);
    strncpy(controller.idstring, idstring, sizeof(controller.idstring));
    rc = DIDDocumentBuilder_AddPublicKey(builder, id3, &controller, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDDocumentBuilder_AddAuthorizationKey(builder, id3, NULL, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *id4 = DIDURL_NewByDid(did, "test4");
    CU_ASSERT_PTR_NOT_NULL(id4);
    dkey = Generater_KeyPair(&_dkey);
    keybase = DerivedKey_GetPublicKeyBase58(dkey, publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    idstring = DerivedKey_GetAddress(dkey);
    CU_ASSERT_PTR_NOT_NULL(idstring);
    strncpy(controller.idstring, idstring, sizeof(controller.idstring));
    rc = DIDDocumentBuilder_AddAuthorizationKey(builder, id4, &controller, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Try to add a non existing key, should fail.
    DIDURL *id = DIDURL_NewByDid(did, "notExistKey");
    CU_ASSERT_PTR_NOT_NULL(id);
    rc = DIDDocumentBuilder_AddAuthorizationKey(builder, id, NULL, NULL);
    CU_ASSERT_EQUAL(rc, -1);
    DIDURL_Destroy(id);

    // Try to add a key not owned by self, should fail.
    id = DIDURL_NewByDid(did, "key2");
    CU_ASSERT_PTR_NOT_NULL(id);
    rc = DIDDocumentBuilder_AddAuthorizationKey(builder, id, NULL, NULL);
    CU_ASSERT_EQUAL(rc, -1);
    DIDURL_Destroy(id);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    // Check existence
    PublicKey *pk = DIDDocument_GetPublicKey(sealeddoc, id1);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id1, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id1);

    pk = DIDDocument_GetPublicKey(sealeddoc, id2);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id2, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id2);

    pk = DIDDocument_GetPublicKey(sealeddoc, id3);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id3, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id3);

    pk = DIDDocument_GetPublicKey(sealeddoc, id4);
    CU_ASSERT_PTR_NOT_NULL(pk);
    isEquals = DIDURL_Equals(id4, PublicKey_GetId(pk));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id4);

    // Check the final count.
    CU_ASSERT_EQUAL(8, DIDDocument_GetPublicKeyCount(sealeddoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(sealeddoc));
    CU_ASSERT_EQUAL(5, DIDDocument_GetAuthorizationCount(sealeddoc));

    DIDDocument_Destroy(sealeddoc);
}

static void test_diddoc_remove_authorization_key(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    DerivedKey _dkey, *dkey;
    const char *keybase, *idstring;
    DID controller;
    int rc;

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    // Add 2 public keys
    DIDURL *id1 = DIDURL_NewByDid(did, "test1");
    CU_ASSERT_PTR_NOT_NULL(id1);
    dkey = Generater_KeyPair(&_dkey);
    keybase = DerivedKey_GetPublicKeyBase58(dkey, publickeybase58,
            sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    idstring = DerivedKey_GetAddress(dkey);
    CU_ASSERT_PTR_NOT_NULL(idstring);
    strncpy(controller.idstring, idstring, sizeof(controller.idstring));
    rc = DIDDocumentBuilder_AddAuthorizationKey(builder, id1, &controller, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *id2 = DIDURL_NewByDid(did, "test2");
    CU_ASSERT_PTR_NOT_NULL(id2);
    dkey = Generater_KeyPair(&_dkey);
    keybase = DerivedKey_GetPublicKeyBase58(dkey, publickeybase58,
            sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    idstring = DerivedKey_GetAddress(dkey);
    CU_ASSERT_PTR_NOT_NULL(idstring);
    strncpy(controller.idstring, idstring, sizeof(controller.idstring));
    rc = DIDDocumentBuilder_AddAuthorizationKey(builder, id2, &controller, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Remote keys
    rc = DIDDocumentBuilder_RemoveAuthorizationKey(builder, id1);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *recoveryid = DIDURL_NewByDid(did, "recovery");
    CU_ASSERT_PTR_NOT_NULL(recoveryid);
    rc = DIDDocumentBuilder_RemoveAuthorizationKey(builder, recoveryid);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Key not exist, should fail.
    DIDURL *id = DIDURL_NewByDid(did, "notExistKey");
    CU_ASSERT_PTR_NOT_NULL(id);
    rc = DIDDocumentBuilder_RemoveAuthorizationKey(builder, id);
    CU_ASSERT_EQUAL(rc, -1);
    DIDURL_Destroy(id);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    // Check existence
    PublicKey *pk = DIDDocument_GetAuthorizationKey(sealeddoc, id1);
    CU_ASSERT_PTR_NULL(pk);
    DIDURL_Destroy(id1);

    pk = DIDDocument_GetAuthorizationKey(sealeddoc, id2);
    CU_ASSERT_PTR_NOT_NULL(pk);
    DIDURL_Destroy(id2);

    pk = DIDDocument_GetAuthorizationKey(sealeddoc, recoveryid);
    CU_ASSERT_PTR_NULL(pk);
    DIDURL_Destroy(recoveryid);

    // Check the final count.
    CU_ASSERT_EQUAL(6, DIDDocument_GetPublicKeyCount(sealeddoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(sealeddoc));
    CU_ASSERT_EQUAL(1, DIDDocument_GetAuthorizationCount(sealeddoc));

    DIDDocument_Destroy(sealeddoc);
}

static void test_diddoc_get_credential(void)
{
    Credential *vcs[2];
    ssize_t size;
    Credential *vc;
    DIDURL *id;
    bool isEquals;

    CU_ASSERT_EQUAL(2, DIDDocument_GetCredentialCount(doc));

    size = DIDDocument_GetCredentials(doc, vcs, sizeof(vcs));
    CU_ASSERT_EQUAL(2, size);

    for (int i = 0; i < size; i++) {
        vc = vcs[i];

        id = Credential_GetId(vc);
        isEquals = DID_Equals(did, &id->did);
        CU_ASSERT_TRUE(isEquals);

        isEquals = DID_Equals(did, Credential_GetOwner(vc));
        CU_ASSERT_TRUE(isEquals);

        CU_ASSERT_TRUE(!strcmp(id->fragment, "profile") || !strcmp(id->fragment, "email"));
    }

    // Credential getter.
    DIDURL *profileid = DIDURL_NewByDid(did, "profile");
    CU_ASSERT_PTR_NOT_NULL(profileid);
    vc = DIDDocument_GetCredential(doc, profileid);
    CU_ASSERT_PTR_NOT_NULL(vc);
    isEquals = DIDURL_Equals(profileid, Credential_GetId(vc));
    CU_ASSERT_TRUE(isEquals);

    id = DIDURL_NewByDid(did, "email");
    CU_ASSERT_PTR_NOT_NULL(id);
    vc = DIDDocument_GetCredential(doc, id);
    CU_ASSERT_PTR_NOT_NULL(vc);
    isEquals = DIDURL_Equals(id, Credential_GetId(vc));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id);

    // Credential not exist.
    id = DIDURL_NewByDid(did, "notExist");
    CU_ASSERT_PTR_NOT_NULL(id);
    vc = DIDDocument_GetCredential(doc, id);
    CU_ASSERT_PTR_NULL(vc);
    DIDURL_Destroy(id);

    // Credential selector.
    size = DIDDocument_SelectCredentials(doc, "SelfProclaimedCredential",
            profileid, vcs, sizeof(vcs));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(Credential_GetId(vcs[0]), profileid);
    CU_ASSERT_TRUE(isEquals);

    size = DIDDocument_SelectCredentials(doc, NULL, profileid, vcs, sizeof(vcs));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(Credential_GetId(vcs[0]), profileid);
    CU_ASSERT_TRUE(isEquals);

    size = DIDDocument_SelectCredentials(doc, "SelfProclaimedCredential",
            NULL, vcs, sizeof(vcs));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(Credential_GetId(vcs[0]), profileid);
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(profileid);

    size = DIDDocument_SelectCredentials(doc, "TestingCredential", NULL, vcs, sizeof(vcs));
    CU_ASSERT_EQUAL(size, 0);
}

static void test_diddoc_add_credential(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    Credential *vc;
    bool isEquals;
    int rc;

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    // Add credentials.
    rc = DIDDocumentBuilder_AddCredential(builder, TestData_LoadPassportVc());
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = DIDDocumentBuilder_AddCredential(builder, TestData_LoadTwitterVc());
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Credential already exist, should fail.
    rc = DIDDocumentBuilder_AddCredential(builder, TestData_LoadTwitterVc());
    CU_ASSERT_EQUAL(rc, -1);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    // Check new added credential.
    DIDURL *id = DIDURL_NewByDid(did, "passport");
    CU_ASSERT_PTR_NOT_NULL(id);
    vc = DIDDocument_GetCredential(sealeddoc, id);
    CU_ASSERT_PTR_NOT_NULL(vc);
    isEquals = DIDURL_Equals(id, Credential_GetId(vc));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(did, "twitter");
    CU_ASSERT_PTR_NOT_NULL(id);
    vc = DIDDocument_GetCredential(sealeddoc, id);
    CU_ASSERT_PTR_NOT_NULL(vc);
    isEquals = DIDURL_Equals(id, Credential_GetId(vc));
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id);

    // Check the final count.
    CU_ASSERT_EQUAL(4, DIDDocument_GetCredentialCount(sealeddoc));

    DIDDocument_Destroy(sealeddoc);
}

static void test_diddoc_add_selfclaimed_credential(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    Credential *vc;
    bool isEquals;
    int rc;
    const char *provalue;

    // Add self claim credential.
    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    DIDURL *credid = DIDURL_NewByDid(did, "passport");
    CU_ASSERT_PTR_NOT_NULL(credid);

    const char *types[] = {"BasicProfileCredential", "SelfProclaimedCredential"};
    Property props[2];
    props[0].key = "nation";
    props[0].value = "Singapore";
    props[1].key = "passport";
    props[1].value = "S653258Z07";

    rc = DIDDocumentBuilder_AddSelfClaimedCredential(builder, credid,
            types, 2, props, 2, DIDDocument_GetExpires(doc), storepass);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    // check credential
    vc = DIDDocument_GetCredential(sealeddoc, credid);
    CU_ASSERT_PTR_NOT_NULL(vc);
    CU_ASSERT_TRUE(Credential_IsSelfProclaimed(vc));
    CU_ASSERT_EQUAL(Credential_GetTypeCount(vc), 2);
    CU_ASSERT_EQUAL(Credential_GetPropertyCount(vc), 2);
    provalue = Credential_GetProperty(vc, "passport");
    CU_ASSERT_STRING_EQUAL(provalue, "S653258Z07");
    free((char*)provalue);

    const char *types1[2];
    rc = Credential_GetTypes(vc, types1, sizeof(types1));
    CU_ASSERT_NOT_EQUAL(rc, -1);

    for (int i = 0; i < 2; i++) {
        const char *type = types1[i];
        CU_ASSERT_TRUE(!strcmp(type, "BasicProfileCredential") ||
                !strcmp(type, "SelfProclaimedCredential"));
    }

    DIDURL_Destroy(credid);
    DIDDocument_Destroy(sealeddoc);
}

static void test_diddoc_remove_credential(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    Credential *vc;
    int rc;

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    // Add credentials.
    rc = DIDDocumentBuilder_AddCredential(builder, TestData_LoadPassportVc());
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = DIDDocumentBuilder_AddCredential(builder, TestData_LoadTwitterVc());
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *profileid = DIDURL_NewByDid(did, "profile");
    CU_ASSERT_PTR_NOT_NULL(profileid);
    rc = DIDDocumentBuilder_RemoveCredential(builder, profileid);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *twitterid = DIDURL_NewByDid(did, "twitter");
    CU_ASSERT_PTR_NOT_NULL(twitterid);
    rc = DIDDocumentBuilder_RemoveCredential(builder, twitterid);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *id = DIDURL_NewByDid(did, "notExistCredential");
    CU_ASSERT_PTR_NOT_NULL(id);
    rc = DIDDocumentBuilder_RemoveCredential(builder, id);
    CU_ASSERT_EQUAL(rc, -1);
    DIDURL_Destroy(id);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    // Check existence
    vc = DIDDocument_GetCredential(sealeddoc, profileid);
    CU_ASSERT_PTR_NULL(vc);
    DIDURL_Destroy(profileid);

    vc = DIDDocument_GetCredential(sealeddoc, twitterid);
    CU_ASSERT_PTR_NULL(vc);
    DIDURL_Destroy(twitterid);

    // Check the final count.
    CU_ASSERT_EQUAL(2, DIDDocument_GetCredentialCount(sealeddoc));

    DIDDocument_Destroy(sealeddoc);
}

static void test_diddoc_get_service(void)
{
    Service *services[3];
    ssize_t size;
    Service *service;
    bool isEquals;

    CU_ASSERT_EQUAL(3, DIDDocument_GetServiceCount(doc));

    size = DIDDocument_GetServices(doc, services, sizeof(services));
    CU_ASSERT_EQUAL(3, size);

    for (int i = 0; i < size; i++) {
        service = services[i];

        DIDURL *id = Service_GetId(service);
        isEquals = DID_Equals(did, &id->did);
        CU_ASSERT_TRUE(isEquals);

        CU_ASSERT_TRUE(!strcmp(id->fragment, "openid") ||
                !strcmp(id->fragment, "vcr") || !strcmp(id->fragment, "carrier"));
    }

    // Service getter, should success.
    DIDURL *openid = DIDURL_NewByDid(did, "openid");
    CU_ASSERT_PTR_NOT_NULL(openid);
    service = DIDDocument_GetService(doc, openid);
    CU_ASSERT_PTR_NOT_NULL(service);
    isEquals = DIDURL_Equals(openid, Service_GetId(service));
    CU_ASSERT_TRUE(isEquals);
    CU_ASSERT_STRING_EQUAL("OpenIdConnectVersion1.0Service", Service_GetType(service));
    CU_ASSERT_STRING_EQUAL("https://openid.example.com/", Service_GetEndpoint(service));

    DIDURL *vcrid = DIDURL_NewByDid(did, "vcr");
    CU_ASSERT_PTR_NOT_NULL(vcrid);
    service = DIDDocument_GetService(doc, vcrid);
    CU_ASSERT_PTR_NOT_NULL(service);
    isEquals = DIDURL_Equals(vcrid, Service_GetId(service));
    CU_ASSERT_TRUE(isEquals);

    // Service not exist, should fail.
    DIDURL *notexistid = DIDURL_NewByDid(did, "notExistService");
    CU_ASSERT_PTR_NOT_NULL(notexistid);
    service = DIDDocument_GetService(doc, notexistid);
    CU_ASSERT_PTR_NULL(service);

    // Service selector.
    size = DIDDocument_SelectServices(doc, "CredentialRepositoryService", vcrid,
            services, sizeof(services));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(Service_GetId(services[0]), vcrid);
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(vcrid);

    size = DIDDocument_SelectServices(doc, NULL, openid, services, sizeof(services));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(Service_GetId(services[0]), openid);
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(openid);

    DIDURL *id = DIDURL_NewByDid(did, "carrier");
    CU_ASSERT_PTR_NOT_NULL(id);
    size = DIDDocument_SelectServices(doc, "CarrierAddress", NULL, services, sizeof(services));
    CU_ASSERT_EQUAL(size, 1);
    isEquals = DIDURL_Equals(Service_GetId(services[0]), id);
    CU_ASSERT_TRUE(isEquals);
    DIDURL_Destroy(id);

    // Service not exist, should return a empty list.
    size = DIDDocument_SelectServices(doc, "CredentialRepositoryService",
            notexistid, services, sizeof(services));
    CU_ASSERT_EQUAL(size, 0);
    DIDURL_Destroy(notexistid);

    size = DIDDocument_SelectServices(doc, "notExistType", NULL, services, sizeof(services));
    CU_ASSERT_EQUAL(size, 0);
}

static void test_diddoc_add_service(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    Service *services[3];
    Service *service;
    ssize_t size;
    int rc;

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    // Add credentials.
    DIDURL *id1 = DIDURL_NewByDid(did, "test-svc-1");
    CU_ASSERT_PTR_NOT_NULL(id1);
    rc = DIDDocumentBuilder_AddService(builder, id1, "Service.Testing",
            "https://www.elastos.org/testing1");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *id2 = DIDURL_NewByDid(did, "test-svc-2");
    CU_ASSERT_PTR_NOT_NULL(id2);
    rc = DIDDocumentBuilder_AddService(builder, id2, "Service.Testing",
            "https://www.elastos.org/testing2");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Service id already exist, should failed.
    DIDURL *id = DIDURL_NewByDid(did, "vcr");
    CU_ASSERT_PTR_NOT_NULL(id1);
    rc = DIDDocumentBuilder_AddService(builder, id, "test", "https://www.elastos.org/test");
    CU_ASSERT_EQUAL(rc, -1);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    //  Check the final count
    CU_ASSERT_EQUAL(5, DIDDocument_GetServiceCount(sealeddoc));

    // Try to select new added 2 services
    size = DIDDocument_SelectServices(sealeddoc, "Service.Testing", NULL,
            services, sizeof(services));
    CU_ASSERT_EQUAL(2, size);
    CU_ASSERT_STRING_EQUAL("Service.Testing", Service_GetType(services[0]));
    CU_ASSERT_STRING_EQUAL("Service.Testing", Service_GetType(services[1]));

    // Check the final count.
    CU_ASSERT_EQUAL(5, DIDDocument_GetServiceCount(sealeddoc));

    DIDDocument_Destroy(sealeddoc);
}

static void test_diddoc_remove_service(void)
{
    DIDDocument *sealeddoc;
    DIDDocumentBuilder *builder;
    int rc;

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    // remove services
    DIDURL *openid = DIDURL_NewByDid(did, "openid");
    CU_ASSERT_PTR_NOT_NULL(openid);
    rc = DIDDocumentBuilder_RemoveService(builder, openid);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDURL *vcrid = DIDURL_NewByDid(did, "vcr");
    CU_ASSERT_PTR_NOT_NULL(vcrid);
    rc = DIDDocumentBuilder_RemoveService(builder, vcrid);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    // Service not exist, should fail.
    DIDURL *id = DIDURL_NewByDid(did, "notExistService");
    CU_ASSERT_PTR_NOT_NULL(id);
    rc = DIDDocumentBuilder_RemoveService(builder, id);
    CU_ASSERT_EQUAL(rc, -1);
    DIDURL_Destroy(id);

    sealeddoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(sealeddoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(sealeddoc));
    DIDDocumentBuilder_Destroy(builder);

    // Check existence
    Service *service = DIDDocument_GetService(sealeddoc, openid);
    CU_ASSERT_PTR_NULL(service);
    DIDURL_Destroy(openid);

    service = DIDDocument_GetService(sealeddoc, vcrid);
    CU_ASSERT_PTR_NULL(service);
    DIDURL_Destroy(vcrid);

    // Check the final count.
    CU_ASSERT_EQUAL(1, DIDDocument_GetServiceCount(sealeddoc));

    DIDDocument_Destroy(sealeddoc);
}

static int diddoc_elem_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    int rc;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(false, storePath);
    if (!store)
        return -1;

    rc = TestData_InitIdentity(store);
    if (rc) {
        TestData_Free();
        return -1;
    }

    doc = TestData_LoadDoc();
    if (!doc || !DIDDocument_IsValid(doc)) {
        TestData_Free();
        return -1;
    }

    did = DIDDocument_GetSubject(doc);
    if (!did) {
        TestData_Free();
        return -1;
    }

    return  0;
}

static int diddoc_elem_test_suite_cleanup(void)
{
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_diddoc_get_publickey",                 test_diddoc_get_publickey             },
    { "test_diddoc_add_publickey",                 test_diddoc_add_publickey             },
    { "test_diddoc_remove_publickey",              test_diddoc_remove_publickey          },
    { "test_diddoc_get_authentication_key",        test_diddoc_get_authentication_key    },
    { "test_diddoc_add_authentication_key",        test_diddoc_add_authentication_key    },
    { "test_diddoc_remove_authentication_key",     test_diddoc_remove_authentication_key },
    { "test_diddoc_get_authorization_key",         test_diddoc_get_authorization_key     },
    { "test_diddoc_add_authorization_key",         test_diddoc_add_authorization_key     },
    { "test_diddoc_remove_authorization_key",      test_diddoc_remove_authorization_key  },
    { "test_diddoc_get_credential",                test_diddoc_get_credential            },
    { "test_diddoc_add_credential",                test_diddoc_add_credential            },
    { "test_diddoc_add_selfclaimed_credential",    test_diddoc_add_selfclaimed_credential},
    { "test_diddoc_remove_credential",             test_diddoc_remove_credential         },
    { "test_diddoc_get_service",                   test_diddoc_get_service               },
    { "test_diddoc_add_service",                   test_diddoc_add_service               },
    { "test_diddoc_remove_service",                test_diddoc_remove_service            },
    { NULL,                                        NULL                                  }
};

static CU_SuiteInfo suite[] = {
    { "diddoc elem test",  diddoc_elem_test_suite_init,  diddoc_elem_test_suite_cleanup,  NULL, NULL, cases },
    {  NULL,               NULL,                         NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* diddoc_elem_test_suite_info(void)
{
    return suite;
}
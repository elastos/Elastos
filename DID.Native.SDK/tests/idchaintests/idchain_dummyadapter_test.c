#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <CUnit/Basic.h>
#include <limits.h>

#include "ela_did.h"
#include "constant.h"
#include "loader.h"
#include "did.h"
#include "didmeta.h"
#include "didstore.h"
#include "diddocument.h"

#define MAX_PUBLICKEY_BASE58      64
#define MAX_DOC_SIGN              128

static DIDStore *store;

static void test_idchain_publishdid(void)
{
    DIDURL *signkey;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN];
    char *signs[3];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish", *sign;
    bool successed;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    signkey = DIDDocument_GetDefaultPublicKey(doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    DID_Copy(&did, DIDDocument_GetSubject(doc));

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, signkey, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n-- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);

    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);

    sign = DIDDocument_GetProofSignature(doc);
    CU_ASSERT_STRING_EQUAL(DIDDocument_GetProofSignature(doc), DIDDocument_GetProofSignature(resolvedoc));
    signs[0] = alloca(strlen(sign) + 1);
    strcpy(signs[0], sign);

    DIDDocument_Destroy(doc);
    printf("\n   txid = %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);

    const char *nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);

    sign = DIDDocument_GetProofSignature(doc);
    signs[1] = alloca(strlen(sign) + 1);
    strcpy(signs[1], sign);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n-- resolve begin(update)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        sleep(30);
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            break;
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    strcpy(previous_txid, txid);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(resolvedoc));
    printf("\n   txid = %s\n-- resolve result: successfully!\n-- publish begin(update) again, waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update again
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    keyid = DIDURL_NewByDid(&did, "key2");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    sign = DIDDocument_GetProofSignature(doc);
    signs[2] = alloca(strlen(sign) + 1);
    strcpy(signs[2], sign);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n-- resolve begin(update) again", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        sleep(30);
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            break;
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(resolvedoc));

    printf("\n   txid = %s\n-- resolve result: successfully!\n------------------------------------------------------------\n", txid);
    DIDDocument_Destroy(resolvedoc);

    //didhistory
    DIDHistory *history = DID_ResolveHistory(&did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(history);
    CU_ASSERT_EQUAL(3, DIDHistory_GetTransactionCount(history));
    CU_ASSERT_EQUAL(0, DIDHistory_GetStatus(history));

    DID *owner = DIDHistory_GetOwner(history);
    CU_ASSERT_PTR_NOT_NULL_FATAL(owner);
    bool bEqual = DID_Equals(&did, owner);
    CU_ASSERT_TRUE_FATAL(bEqual);

    for (i = 0; i < 3; i++) {
        doc = DIDHistory_GetDocumentByIndex(history, i);
        CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
        CU_ASSERT_STRING_EQUAL(signs[2-i], DIDDocument_GetProofSignature(doc));
        DIDDocument_Destroy(doc);
    }
    DIDHistory_Destroy(history);
}

static void test_idchain_publishdid_without_txid(void)
{
    DIDURL *signkey;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    bool successed;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    signkey = DIDDocument_GetDefaultPublicKey(doc);
    CU_ASSERT_PTR_NOT_NULL(signkey);

    DID_Copy(&did, DIDDocument_GetSubject(doc));

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, signkey, false);
    CU_ASSERT_TRUE_FATAL(successed);
    DIDDocument_Destroy(doc);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }

    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(previous_txid, txid);

    printf("\n   txid = %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);

    rc = DIDMetaData_SetTxid(metadata, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    const char *nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_STRING_EQUAL(txid, "");
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    strcpy(previous_txid, txid);

    printf("\n   txid = %s\n-- resolve result: successfully!\n-- publish begin(update) again, waiting...\n", txid);
    metadata = DIDDocument_GetMetaData(resolvedoc);
    rc = DIDMetaData_SetTxid(metadata, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update again
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    keyid = DIDURL_NewByDid(&did, "key2");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update) again", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(resolvedoc));

    printf("\n   txid: %s\n-- resolve result: successfully!\n------------------------------------------------------------\n", txid);
    DIDDocument_Destroy(resolvedoc);
}

static void test_idchain_publishdid_without_signature(void)
{
    DIDURL *signkey;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    bool successed;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    signkey = DIDDocument_GetDefaultPublicKey(doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    DID_Copy(&did, DIDDocument_GetSubject(doc));

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, signkey, false);
    CU_ASSERT_TRUE_FATAL(successed);
    DIDDocument_Destroy(doc);

    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(previous_txid, txid);

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    const char *nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    strcpy(previous_txid, txid);

    rc = DIDMetaData_SetPrevSignature(metadata, resolvedoc->proof.signatureValue);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDMetaData_SetSignature(metadata, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update) again, waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update again
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    keyid = DIDURL_NewByDid(&did, "key2");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update) again", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(resolvedoc));

    printf("\n   txid: %s\n-- resolve result: successfully!\n------------------------------------------------------------\n", txid);
    DIDDocument_Destroy(resolvedoc);
}

static void test_idchain_publishdid_without_prevsignature(void)
{
    DIDURL *signkey;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    bool successed;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    signkey = DIDDocument_GetDefaultPublicKey(doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    DID_Copy(&did, DIDDocument_GetSubject(doc));

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, signkey, false);
    CU_ASSERT_TRUE_FATAL(successed);
    DIDDocument_Destroy(doc);

    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(previous_txid, txid);

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    const char *nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    strcpy(previous_txid, txid);

    rc = DIDMetaData_SetPrevSignature(metadata, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update) again, waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update again
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    keyid = DIDURL_NewByDid(&did, "key2");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);
    const char *signature = DIDMetaData_GetPrevSignature(metadata);
    CU_ASSERT_PTR_NOT_NULL(signature);
    CU_ASSERT_STRING_EQUAL(signature, "");
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update) again", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(resolvedoc));

    printf("\n   txid: %s\n-- resolve result: successfully!\n------------------------------------------------------------\n", txid);
    DIDDocument_Destroy(resolvedoc);
}

static void test_idchain_publishdid_without_prevsignature_and_signature(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN], _path[PATH_MAX];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    bool successed;
    char *path;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(previous_txid, txid);

    rc = DIDMetaData_SetSignature(metadata, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDMetaData_SetPrevSignature(metadata, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_FALSE(successed);
    CU_ASSERT_STRING_EQUAL("Missing signatures information, DID SDK dosen't know how to handle it, use force mode to ignore checks.",
           DIDError_GetMessage());
}

static void test_force_updatedid_without_prevsignature_and_signature(void)
{
    DIDURL *signkey;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    bool successed;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);

    rc = DIDMetaData_SetSignature(metadata, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDMetaData_SetPrevSignature(metadata, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    const char *nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, true);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }
        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(resolvedoc));

    printf("\n   txid: %s\n-- resolve result: successfully!\n------------------------------------------------------------\n", txid);
    DIDDocument_Destroy(resolvedoc);
}

static void test_updatedid_with_diffprevsignature_only(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN], _path[PATH_MAX];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    bool successed;
    char *path;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(previous_txid, txid);

    rc = DIDMetaData_SetPrevSignature(metadata, "123456789");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n-- resolve begin(update)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }

    CU_ASSERT_NOT_EQUAL(previous_txid, txid);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(resolvedoc));

    DIDDocument_Destroy(resolvedoc);
}

static void test_updatedid_with_diffsignature_only(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN], _path[PATH_MAX];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    bool successed;
    char *path;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n    txid: %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    strcpy(previous_txid, txid);

    rc = DIDMetaData_SetPrevSignature(metadata, resolvedoc->proof.signatureValue);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDMetaData_SetSignature(metadata, "123456789");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update) again, waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update again
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    keyid = DIDURL_NewByDid(&did, "key2");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    const char *nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update) again", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(resolvedoc));

    DIDDocument_Destroy(resolvedoc);
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
}

static void test_updatedid_with_diff_prevsignature_and_signature(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN], _path[PATH_MAX];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    bool successed;
    char *path;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(previous_txid, txid);

    rc = DIDMetaData_SetSignature(metadata, "12345678");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDMetaData_SetPrevSignature(metadata, "12345678");
    CU_ASSERT_NOT_EQUAL(rc, -1);


    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_FALSE(successed);
    CU_ASSERT_STRING_EQUAL("Current copy not based on the lastest on-chain copy.",
            DIDError_GetMessage());
}

static void test_force_updatedid_with_wrongsignature(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    bool successed;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(previous_txid, txid);

    rc = DIDMetaData_SetSignature(metadata, "12345678");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n  txid: %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    const char *nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, true);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(resolvedoc));

    DIDDocument_Destroy(resolvedoc);
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
}

static void test_idchain_publishdid_with_credential(void)
{
    DIDDocument *resolvedoc = NULL, *doc;
    char previous_txid[ELA_MAX_TXID_LEN];
    const char *mnemonic, *txid;
    Credential *cred;
    bool successed;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    doc = DIDStore_NewDID(store, storepass, "littlefish");
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(previous_txid, txid);

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    DIDURL *credid = DIDURL_NewByDid(&did, "cred-1");
    CU_ASSERT_PTR_NOT_NULL(credid);

    const char *types[] = {"BasicProfileCredential", "SelfClaimedCredential"};

    Property props[1];
    props[0].key = "name";
    props[0].value = "John";

    rc = DIDDocumentBuilder_AddSelfClaimedCredential(builder, credid, types, 2, props, 1, 0, storepass);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    cred = DIDDocument_GetCredential(doc, credid);
    CU_ASSERT_PTR_NOT_NULL(cred);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, true);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }

    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");

    cred = DIDDocument_GetCredential(resolvedoc, credid);
    CU_ASSERT_PTR_NOT_NULL(cred);

    DIDURL_Destroy(credid);
    DIDDocument_Destroy(resolvedoc);
}

static void test_idchain_deactivedid_after_create(void)
{
    DIDDocument *resolvedoc = NULL, *doc;
    char previous_txid[ELA_MAX_TXID_LEN];
    const char *mnemonic, *txid;
    Credential *cred;
    bool successed;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    doc = DIDStore_NewDID(store, storepass, "littlefish");
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&did, DIDDocument_GetSubject(doc));

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    const char *data1 = DIDDocument_ToJson(doc, true);
    const char *data2 = DIDDocument_ToJson(resolvedoc, true);
    CU_ASSERT_STRING_EQUAL(data1, data2);
    free((void*)data1);
    free((void*)data2);
    strcpy(previous_txid, txid);

    DIDDocument_Destroy(doc);

    successed = DIDStore_DeactivateDID(store, storepass, &did, NULL);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("\n-- deactive did result:\n   did = %s\n -- resolve begin(deactive)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            break;
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
            sleep(30);
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
    CU_ASSERT_STRING_EQUAL("DID is deactivated.", DIDError_GetMessage());

    if (resolvedoc)
        DIDDocument_Destroy(resolvedoc);
    return;
}

static void test_idchain_deactivedid_after_update(void)
{
    DIDURL *signkey;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    bool successed;
    DID did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    //create
    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    signkey = DIDDocument_GetDefaultPublicKey(doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    DID_Copy(&did, DIDDocument_GetSubject(doc));

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, signkey, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);

    metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    const char *nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);

    CU_ASSERT_STRING_EQUAL(DIDDocument_GetProofSignature(doc), DIDDocument_GetProofSignature(resolvedoc));
    DIDDocument_Destroy(doc);

    printf("\n   txid: %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    //update
    doc = DIDStore_LoadDID(store, &did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    keybase = Generater_Publickey(publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);
    DIDURL *keyid = DIDURL_NewByDid(&did, "key1");
    CU_ASSERT_PTR_NOT_NULL(keyid);
    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));
    DIDDocumentBuilder_Destroy(builder);

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    metadata = DIDDocument_GetMetaData(doc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    nalias = DIDMetaData_GetAlias(metadata);
    CU_ASSERT_PTR_NOT_NULL(nalias);
    CU_ASSERT_STRING_EQUAL(alias, nalias);
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    strcpy(previous_txid, txid);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(resolvedoc));
    printf("\n-- resolve result: successfully!\n-- deactive did begin, waiting...\n");

    successed = DIDStore_DeactivateDID(store, storepass, &did, NULL);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- deactive did result:\n   did = %s\n -- resolve begin(deactive)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            break;
        }
        else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
            sleep(30);
        }

        if (++i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }

    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
    CU_ASSERT_STRING_EQUAL("DID is deactivated.", DIDError_GetMessage());

    if (resolvedoc)
        DIDDocument_Destroy(resolvedoc);

    return;
}

static void test_idchain_deactivedid_with_authorization1(void)
{
    char previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc, *targetdoc;
    const char *mnemonic, *txid, *alias = "littlefish";
    DID controller, did;
    PublicKey *pks[1];
    bool isEqual, successed;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&controller, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    printf("\n------------------------------------------------------------\n-- publish authorization did begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &controller, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", controller.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&controller, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL(txid);

    printf("\n   txid: %s\n-- resolve authorization result: successfully!\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    targetdoc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(targetdoc);

    DID_Copy(&did, DIDDocument_GetSubject(targetdoc));

    DIDDocumentBuilder *builder = DIDDocument_Edit(targetdoc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(targetdoc);

    DIDURL *keyid = DIDURL_NewByDid(&did, "recovery");
    CU_ASSERT_PTR_NOT_NULL(keyid);

    rc = DIDDocumentBuilder_AuthorizationDid(builder, keyid, &controller, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    targetdoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(targetdoc);
    DIDDocumentBuilder_Destroy(builder);
    CU_ASSERT_EQUAL(1, DIDDocument_GetAuthorizationCount(targetdoc));

    size_t size = DIDDocument_GetAuthorizationKeys(targetdoc, pks, sizeof(pks));
    CU_ASSERT_EQUAL(1, size);
    isEqual = DID_Equals(&did, &pks[0]->id.did);
    CU_ASSERT_TRUE(isEqual);

    rc = DIDStore_StoreDID(store, targetdoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(targetdoc);

    printf("-- publish target did begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    i = 0;
    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    printf("\n-- resolve authorization result: successfully!\n");

    successed = DIDStore_DeactivateDID(store, storepass, &did, NULL);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- deactive did result:\n   did = %s\n -- resolve begin(deactive)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc)
            break;
        else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }

    printf("\n-- resolve target result: successfully!\n------------------------------------------------------------\n");
    CU_ASSERT_STRING_EQUAL("DID is deactivated.", DIDError_GetMessage());
    if (resolvedoc)
        DIDDocument_Destroy(resolvedoc);
    return;
}

static void test_idchain_deactivedid_with_authorization2(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc, *targetdoc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    HDKey _dkey, *dkey;
    DID controller, did;
    PublicKey *pks[1];
    bool isEqual, successed;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    Mnemonic_Free((void*)mnemonic);

    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&controller, DIDDocument_GetSubject(doc));

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(builder);
    DIDDocument_Destroy(doc);

    dkey = Generater_KeyPair(&_dkey);
    keybase = HDKey_GetPublicKeyBase58(dkey, publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);

    DIDURL *keyid = DIDURL_NewByDid(&controller, "key-2");
    CU_ASSERT_PTR_NOT_NULL(keyid);

    rc = DIDStore_StorePrivateKey(store, storepass, &controller, keyid,
            HDKey_GetPrivateKey(dkey), sizeof(HDKey_GetPrivateKey(dkey)));
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(doc);
    DIDDocumentBuilder_Destroy(builder);

    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));

    rc = DIDStore_StoreDID(store, doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(doc);

    printf("\n------------------------------------------------------------\n-- publish authorization did begin(create), waiting....\n");
    successed = (char *)DIDStore_PublishDID(store, storepass, &controller, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", controller.idstring);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&controller, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);

    printf("\n   txid: %s\n-- resolve authorization result: successfully!\n", txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;

    targetdoc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL(targetdoc);

    builder = DIDDocument_Edit(targetdoc);
    CU_ASSERT_PTR_NOT_NULL(builder);

    DID_Copy(&did, DIDDocument_GetSubject(targetdoc));
    DIDDocument_Destroy(targetdoc);

    keyid = DIDURL_NewByDid(&did, "recovery");
    CU_ASSERT_PTR_NOT_NULL(keyid);

    rc = DIDDocumentBuilder_AddAuthorizationKey(builder, keyid, &controller, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    targetdoc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(targetdoc);
    CU_ASSERT_EQUAL(1, DIDDocument_GetAuthorizationCount(targetdoc));
    DIDDocumentBuilder_Destroy(builder);

    size_t size = DIDDocument_GetAuthorizationKeys(targetdoc, pks, sizeof(pks));
    CU_ASSERT_EQUAL(1, size);
    isEqual = DID_Equals(&did, &pks[0]->id.did);
    CU_ASSERT_TRUE(isEqual);

    rc = DIDStore_StoreDID(store, targetdoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(targetdoc);

    printf("-- publish target did begin(create), waiting....\n");
    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(create)", did.idstring);

    i = 0;
    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }
    metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);
    printf("\n   txid: %s\n-- resolve target result: successfully!", txid);

    successed = DIDStore_DeactivateDID(store, storepass, &did, NULL);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- deactive did result:\n   did = %s\n -- resolve begin(deactive)", did.idstring);

    i = 0;
    txid = previous_txid;
    while(resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            break;
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
            sleep(30);
            if (++i >= 20)
                CU_FAIL_FATAL("publish did timeout!!!!\n");
        }
    }

    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
    CU_ASSERT_STRING_EQUAL("DID is deactivated.", DIDError_GetMessage());
    if (resolvedoc)
        DIDDocument_Destroy(resolvedoc);
    return;
}

static int idchain_dummyadapter_test_suite_init(void)
{
    int rc;
    char _path[PATH_MAX];
    const char *storePath;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(true, storePath);
    if (!store)
        return -1;

    return 0;
}

static int idchain_dummyadapter_test_suite_cleanup(void)
{
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_idchain_publishdid",                                      test_idchain_publishdid                                     },
    { "test_idchain_publishdid_without_txid",                         test_idchain_publishdid_without_txid                        },
    { "test_idchain_publishdid_without_signature",                    test_idchain_publishdid_without_signature                   },
    { "test_idchain_publishdid_without_prevsignature",                test_idchain_publishdid_without_prevsignature               },
    { "test_idchain_publishdid_without_prevsignature_and_signature",  test_idchain_publishdid_without_prevsignature_and_signature },
    { "test_force_updatedid_without_prevsignature_and_signature",     test_force_updatedid_without_prevsignature_and_signature    },
    { "test_updatedid_with_diffprevsignature_only",                   test_updatedid_with_diffprevsignature_only                  },
    { "test_updatedid_with_diffsignature_only",                       test_updatedid_with_diffsignature_only                      },
    { "test_updatedid_with_diff_prevsignature_and_signature",         test_updatedid_with_diff_prevsignature_and_signature        },
    { "test_force_updatedid_with_wrongsignature",                     test_force_updatedid_with_wrongsignature                    },
    { "test_idchain_publishdid_with_credential",                      test_idchain_publishdid_with_credential                     },
    { "test_idchain_deactivedid_after_create",                        test_idchain_deactivedid_after_create                       },
    { "test_idchain_deactivedid_after_update",                        test_idchain_deactivedid_after_update                       },
    { "test_idchain_deactivedid_with_authorization1",                 test_idchain_deactivedid_with_authorization1                },
    { "test_idchain_deactivedid_with_authorization2",                 test_idchain_deactivedid_with_authorization2                },
    {  NULL,                                                          NULL                                                        }
};

static CU_SuiteInfo suite[] = {
    { "idchain dummyadapter test", idchain_dummyadapter_test_suite_init, idchain_dummyadapter_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                      NULL,                              NULL,                                 NULL, NULL, NULL  }
};

CU_SuiteInfo* idchain_dummyadapter_test_suite_info(void)
{
    return suite;
}

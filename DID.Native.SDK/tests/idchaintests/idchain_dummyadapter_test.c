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
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, signkey, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(resolvedoc));
    CU_ASSERT_STRING_EQUAL(DIDDocument_GetProofSignature(doc), DIDDocument_GetProofSignature(resolvedoc));
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

    //update
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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(doc));

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update)", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {
            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(resolvedoc));
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update) again, waiting...\n");

    //update again
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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(doc));
    DIDDocument_Destroy(doc);

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update) again", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {
            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(resolvedoc));

    DIDDocument_Destroy(resolvedoc);
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
}

static void test_idchain_publishdid_without_txid(void)
{
    DIDURL *signkey;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, signkey, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    rc = Set_Doc_Txid(resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

    //update
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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(doc));

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update)", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {
            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    rc = Set_Doc_Txid(resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update) again, waiting...\n");

    //update again
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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(doc));
    DIDDocument_Destroy(doc);

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update) again", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {

            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(resolvedoc));

    DIDDocument_Destroy(resolvedoc);
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
}

static void test_idchain_publishdid_without_signature(void)
{
    DIDURL *signkey;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, signkey, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    rc = Set_Doc_Signature(resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

    //update
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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(doc));

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update)", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {
            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    rc = Set_Doc_Signature(resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update) again, waiting...\n");

    //update again
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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(doc));
    DIDDocument_Destroy(doc);

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update) again", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {

            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(resolvedoc));

    DIDDocument_Destroy(resolvedoc);
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
}

static void test_idchain_publishdid_without_txid_and_signature(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN], _path[PATH_MAX];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    path = get_file_path(_path, PATH_MAX, 7, store->root, PATH_STEP,
                DID_DIR, PATH_STEP, did.idstring, PATH_STEP, META_FILE);
    delete_file(path);

    rc = Set_Doc_Signature(resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = Set_Doc_Txid(resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(doc);

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NULL(txid);
    CU_ASSERT_STRING_EQUAL("Missing last transaction id and signature, use force mode to ignore checks.",
           DIDError_GetMessage());
}

static void test_force_updatedid_without_txid_and_signature(void)
{
    DIDURL *signkey;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    rc = Set_Doc_Signature(resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = Set_Doc_Txid(resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(doc));
    DIDDocument_Destroy(doc);

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, true);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update)", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {

            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);

    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(resolvedoc));

    DIDDocument_Destroy(resolvedoc);
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
}

static void test_updatedid_with_wrongtxid(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN], _path[PATH_MAX];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    path = get_file_path(_path, PATH_MAX, 7, store->root, PATH_STEP,
                DID_DIR, PATH_STEP, did.idstring, PATH_STEP, META_FILE);
    delete_file(path);

    rc = Set_Doc_Txid(resolvedoc, "1234567890");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(doc);

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NULL(txid);
    CU_ASSERT_STRING_EQUAL("Current copy not based on the lastest on-chain copy, txid mismatch.",
            DIDError_GetMessage());
}

static void test_updatedid_with_wrongsignature(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN], _path[PATH_MAX];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    path = get_file_path(_path, PATH_MAX, 7, store->root, PATH_STEP,
                DID_DIR, PATH_STEP, did.idstring, PATH_STEP, META_FILE);
    delete_file(path);

    rc = Set_Doc_Signature(resolvedoc, "1234567890");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(doc);

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NULL(txid);
    CU_ASSERT_STRING_EQUAL("Current copy not based on the lastest on-chain copy, txid mismatch.",
            DIDError_GetMessage());
}

static void test_force_updatedid_with_wrongtxid(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    rc = Set_Doc_Txid(resolvedoc, "1234567890");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(doc));
    DIDDocument_Destroy(doc);

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, true);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update)", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {

            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);

    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(resolvedoc));

    DIDDocument_Destroy(resolvedoc);
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
}

static void test_force_updatedid_with_wrongsignature(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    rc = Set_Doc_Signature(resolvedoc, "1234567890");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(doc));
    DIDDocument_Destroy(doc);

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, true);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update)", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {

            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);

    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(resolvedoc));

    DIDDocument_Destroy(resolvedoc);
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
}

static void test_idchain_publishdid_with_credential(void)
{
    DIDDocument *resolvedoc = NULL, *doc;
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    const char *mnemonic, *txid;
    Credential *cred;
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = NULL;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);

    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    cred = DIDDocument_GetCredential(doc, credid);
    CU_ASSERT_PTR_NOT_NULL(cred);

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, true);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update)", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {
            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }

    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);

    cred = DIDDocument_GetCredential(resolvedoc, credid);
    CU_ASSERT_PTR_NOT_NULL(cred);

    DIDURL_Destroy(credid);
    DIDDocument_Destroy(resolvedoc);
}

static void test_idchain_deactivedid_after_create(void)
{
    DIDDocument *resolvedoc = NULL, *doc;
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    const char *mnemonic, *txid;
    Credential *cred;
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = NULL;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    CU_ASSERT_STRING_EQUAL(DIDDocument_ToJson(doc, true), DIDDocument_ToJson(resolvedoc, true));
    strcpy(previous_txid, txid);

    DIDDocument_Destroy(doc);

    txid = DIDStore_DeactivateDID(store, storepass, &did, NULL);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("\n-- deactive did result:\n   did = %s\n   txid = %s\n-- resolve begin(deactive)", did.idstring, txid);

    while(resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            break;
        }
        else {
            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            sleep(30);
            continue;
        }
    }

    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
    CU_ASSERT_STRING_EQUAL("DID is deactivated.", DIDError_GetMessage());
    return;
}

static void test_idchain_deactivedid_after_update(void)
{
    DIDURL *signkey;
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, signkey, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(txid);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(resolvedoc));
    CU_ASSERT_STRING_EQUAL(DIDDocument_GetProofSignature(doc), DIDDocument_GetProofSignature(resolvedoc));
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n");

    //update
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

    rc = DIDStore_StoreDID(store, doc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(alias, DIDDocument_GetAlias(doc));

    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update)", did.idstring, publish_txid);

    while(!resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {
            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            continue;
        }
    }
    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(txid, publish_txid);
    strcpy(previous_txid, publish_txid);
    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(resolvedoc));
    printf("\n-- resolve result: successfully!\n-- deactive did begin, waiting...\n");

    txid = DIDStore_DeactivateDID(store, storepass, &did, NULL);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- deactive did result:\n   did = %s\n   txid = %s\n-- resolve begin(deactive)", did.idstring, txid);

    while(resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            break;
        }
        else {
            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            sleep(30);
            continue;
        }
    }

    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
    CU_ASSERT_STRING_EQUAL("DID is deactivated.", DIDError_GetMessage());
    return;
}

static void test_idchain_deactivedid_with_authorization1(void)
{
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc, *targetdoc;
    const char *mnemonic, *txid, *alias = "littlefish";
    DID controller, did;
    PublicKey *pks[1];
    bool isEqual;
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
    txid = (char *)DIDStore_PublishDID(store, storepass, &controller, NULL, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", controller.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&controller, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(txid);

    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve authorization result: successfully!\n");

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

    rc = DIDStore_StoreDID(store, targetdoc, alias);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(targetdoc);

    printf("-- publish target did begin(create), waiting....\n");
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);

    rc = DIDStore_StoreDID(store, resolvedoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    printf("\n-- resolve authorization result: successfully!\n");

    txid = DIDStore_DeactivateDID(store, storepass, &did, NULL);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- deactive did result:\n   did = %s\n   txid = %s\n-- resolve begin(deactive)", did.idstring, txid);

    while(resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc)
            break;
        else {
            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            sleep(30);
            continue;
        }
    }

    printf("\n-- resolve target result: successfully!\n------------------------------------------------------------\n");
    CU_ASSERT_STRING_EQUAL("DID is deactivated.", DIDError_GetMessage());
    return;
}

static void test_idchain_deactivedid_with_authorization2(void)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    char publish_txid[ELA_MAX_TXID_LEN], previous_txid[ELA_MAX_TXID_LEN];
    DIDDocument *resolvedoc = NULL, *doc, *targetdoc;
    const char *mnemonic, *txid, *keybase, *alias = "littlefish";
    DerivedKey _dkey, *dkey;
    DID controller, did;
    PublicKey *pks[1];
    bool isEqual;
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
    keybase = DerivedKey_GetPublicKeyBase58(dkey, publickeybase58, sizeof(publickeybase58));
    CU_ASSERT_PTR_NOT_NULL(keybase);

    DIDURL *keyid = DIDURL_NewByDid(&controller, "key-2");
    CU_ASSERT_PTR_NOT_NULL(keyid);

    rc = DIDStore_StorePrivateKey(store, storepass, &controller, keyid,
            DerivedKey_GetPrivateKey(dkey));
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, keybase);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDURL_Destroy(keyid);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(doc);
    DIDDocumentBuilder_Destroy(builder);

    CU_ASSERT_EQUAL(2, DIDDocument_GetPublicKeyCount(doc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetAuthenticationCount(doc));

    rc = DIDStore_StoreDID(store, doc, alias);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(doc);

    printf("\n------------------------------------------------------------\n-- publish authorization did begin(create), waiting....\n");
    txid = (char *)DIDStore_PublishDID(store, storepass, &controller, NULL, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", controller.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&controller, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);
    DIDDocument_Destroy(resolvedoc);
    resolvedoc = NULL;
    printf("\n-- resolve authorization result: successfully!\n");

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

    rc = DIDStore_StoreDID(store, targetdoc, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    DIDDocument_Destroy(targetdoc);

    printf("-- publish target did begin(create), waiting....\n");
    txid = (char *)DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(txid);
    strcpy(publish_txid, txid);
    free((char*)txid);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did.idstring, publish_txid);

    while(!resolvedoc) {
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            ++i;
            printf(".");
            sleep(30);
        }
    }
    txid = DIDDocument_GetTxid(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);
    printf("\n-- resolve target result: successfully!");

    txid = DIDStore_DeactivateDID(store, storepass, &did, NULL);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    strcpy(publish_txid, txid);
    free((char*)txid);
    txid = publish_txid;
    printf("-- deactive did result:\n   did = %s\n   txid = %s\n-- resolve begin(deactive)", did.idstring, txid);

    while(resolvedoc || !strcmp(previous_txid, txid)) {
        if (resolvedoc)
            DIDDocument_Destroy(resolvedoc);

        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            break;
        }
        else {
            txid = DIDDocument_GetTxid(resolvedoc);
            printf(".");
            sleep(30);
            continue;
        }
    }

    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");
    CU_ASSERT_STRING_EQUAL("DID is deactivated.", DIDError_GetMessage());
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
    { "test_idchain_publishdid",                             test_idchain_publishdid                            },
    { "test_idchain_publishdid_without_txid",                test_idchain_publishdid_without_txid               },
    { "test_idchain_publishdid_without_signature",           test_idchain_publishdid_without_signature          },
    { "test_idchain_publishdid_without_txid_and_signature",  test_idchain_publishdid_without_txid_and_signature },
    { "test_force_updatedid_without_txid_and_signature",     test_force_updatedid_without_txid_and_signature    },
    { "test_updatedid_with_wrongtxid",                       test_updatedid_with_wrongtxid                      },
    { "test_updatedid_with_wrongsignature",                  test_updatedid_with_wrongsignature                 },
    { "test_force_updatedid_with_wrongtxid",                 test_force_updatedid_with_wrongtxid                },
    { "test_force_updatedid_with_wrongsignature",            test_force_updatedid_with_wrongsignature           },
    { "test_idchain_publishdid_with_credential",             test_idchain_publishdid_with_credential            },
    { "test_idchain_deactivedid_after_create",               test_idchain_deactivedid_after_create              },
    { "test_idchain_deactivedid_after_update",               test_idchain_deactivedid_after_update              },
    { "test_idchain_deactivedid_with_authorization1",        test_idchain_deactivedid_with_authorization1       },
    { "test_idchain_deactivedid_with_authorization2",        test_idchain_deactivedid_with_authorization2       },
    {  NULL,                                                 NULL                                               }
};

static CU_SuiteInfo suite[] = {
    { "idchain dummyadapter test", idchain_dummyadapter_test_suite_init, idchain_dummyadapter_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                      NULL,                              NULL,                                 NULL, NULL, NULL  }
};

CU_SuiteInfo* idchain_dummyadapter_test_suite_info(void)
{
    return suite;
}

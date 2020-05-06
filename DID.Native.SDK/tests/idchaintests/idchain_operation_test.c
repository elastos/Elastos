#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <crystal.h>
#include <CUnit/Basic.h>
#include <limits.h>

#include "constant.h"
#include "loader.h"
#include "ela_did.h"
#include "did.h"
#include "didmeta.h"
#include "diddocument.h"

static DIDDocument *document;
static DIDStore *store;

static void test_idchain_publishdid_and_resolve(void)
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

static void test_idchain_publishdid_with_credential(void)
{
    DIDURL *signkey;
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

static int idchain_operation_test_suite_init(void)
{
    int rc;
    char _path[PATH_MAX];
    const char *storePath;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(false, storePath);
    if (!store)
        return -1;

    return 0;
}

static int idchain_operation_test_suite_cleanup(void)
{
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_idchain_publishdid_and_resolve",         test_idchain_publishdid_and_resolve          },
    { "test_idchain_publishdid_with_credential",     test_idchain_publishdid_with_credential      },
    {  NULL,                                         NULL                                         }
};

static CU_SuiteInfo suite[] = {
    { "id chain operateion test", idchain_operation_test_suite_init, idchain_operation_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                      NULL,                              NULL,                                 NULL, NULL, NULL  }
};

CU_SuiteInfo* idchain_operation_test_suite_info(void)
{
    return suite;
}
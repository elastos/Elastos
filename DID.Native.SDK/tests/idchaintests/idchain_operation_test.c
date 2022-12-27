#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
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

    CU_ASSERT_STRING_EQUAL(DIDDocument_GetProofSignature(doc), DIDDocument_GetProofSignature(resolvedoc));
    DIDDocument_Destroy(doc);

    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDMetaData *metadata = DIDDocument_GetMetaData(resolvedoc);
    CU_ASSERT_PTR_NOT_NULL(metadata);
    txid = DIDMetaData_GetTxid(metadata);
    CU_ASSERT_PTR_NOT_NULL(txid);
    strcpy(previous_txid, txid);
    printf("\n   txid = %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
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

        sleep(30);
        resolvedoc = DID_Resolve(&did, true);
        if (!resolvedoc) {
            break;
        } else {
            metadata = DIDDocument_GetMetaData(resolvedoc);
            txid = DIDMetaData_GetTxid(metadata);
            printf(".");
        }

        ++i;
        if (i >= 20)
            CU_FAIL_FATAL("publish did timeout!!!!\n");
    }
    rc = DIDStore_StoreDID(store, resolvedoc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_NOT_EQUAL_FATAL(previous_txid, txid);
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
    DIDDocument_Destroy(doc);

    successed = DIDStore_PublishDID(store, storepass, &did, NULL, false);
    CU_ASSERT_TRUE_FATAL(successed);
    printf("-- publish result:\n   did = %s\n -- resolve begin(update) again", did.idstring);

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
    CU_ASSERT_NOT_EQUAL_FATAL(previous_txid, txid);
    CU_ASSERT_EQUAL(3, DIDDocument_GetPublicKeyCount(resolvedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(resolvedoc));

    printf("\n   txid = %s\n-- resolve result: successfully!\n------------------------------------------------------------\n", txid);
    DIDDocument_Destroy(resolvedoc);
}

static void test_idchain_publishdid_with_credential(void)
{
    DIDURL *signkey;
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
            ++i;
            if (i >= 20)
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

    printf("\n   txid = %s\n-- resolve result: successfully!\n-- publish begin(update), waiting...\n", txid);
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

    printf("\n   txid = %s\n-- resolve result: successfully!\n------------------------------------------------------------\n", txid);

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
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

static DIDDocument *document;
static DIDStore *store;

static void test_idchain_publishdid(void)
{
    char *txid;
    DIDURL *signkey;
    DIDDocument *doc = NULL, *updatedoc = NULL, *document;
    const char *createTxid, *updateTxid, *mnemonic;
    DID *did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    document = DIDStore_NewDID(store, storepass, "littlefish");
    CU_ASSERT_PTR_NOT_NULL(document);

    signkey = DIDDocument_GetDefaultPublicKey(document);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    did = DIDDocument_GetSubject(document);
    CU_ASSERT_PTR_NOT_NULL(did);

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    txid = (char *)DIDStore_PublishDID(store, storepass, did, signkey, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did->idstring, txid);

    while(!doc) {
        doc = DID_Resolve(did, true);
        if (!doc) {
            ++i;
            printf(".");
            sleep(30);
        }
        else {
            rc = DIDStore_StoreDID(store, doc, "");
            CU_ASSERT_NOT_EQUAL(rc, -1);
            createTxid = DIDDocument_GetTxid(doc);
            CU_ASSERT_STRING_EQUAL(txid, createTxid);
        }
    }
    free(txid);
    printf("\n-- resolve result: successfully!\n-- publish begin(update), wating...\n");

    signkey = DIDDocument_GetDefaultPublicKey(doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    txid = (char *)DIDStore_PublishDID(store, storepass, did, signkey, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update)", did->idstring, txid);

    while(!updatedoc || !strcmp(createTxid, updateTxid)) {
        if (updatedoc)
            DIDDocument_Destroy(updatedoc);

        updatedoc = DID_Resolve(did, true);
        if (!updatedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {
            rc = DIDStore_StoreDID(store, updatedoc, "");
            CU_ASSERT_NOT_EQUAL(rc, -1);
            updateTxid = DIDDocument_GetTxid(updatedoc);
            printf(".");
            continue;
        }
    }
    CU_ASSERT_STRING_EQUAL(txid, updateTxid);
    free(txid);
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");

    Mnemonic_Free((void*)mnemonic);
    DIDDocument_Destroy(doc);
    DIDDocument_Destroy(updatedoc);
    DIDDocument_Destroy(document);
}

static void test_idchain_publishdid_with_credential(void)
{
    char *txid;
    DIDURL *signkey;
    DIDDocument *doc = NULL, *updatedoc = NULL, *document;
    const char *createTxid, *updateTxid, *mnemonic;
    DID *did;
    int i = 0, rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    document = DIDStore_NewDID(store, storepass, "littlefish");
    CU_ASSERT_PTR_NOT_NULL(document);

    signkey = DIDDocument_GetDefaultPublicKey(document);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    did = DIDDocument_GetSubject(document);
    CU_ASSERT_PTR_NOT_NULL(did);

    printf("\n------------------------------------------------------------\n-- publish begin(create), waiting....\n");
    txid = (char *)DIDStore_PublishDID(store, storepass, did, signkey, false);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(create)", did->idstring, txid);

    while(!doc) {
        doc = DID_Resolve(did, true);
        if (!doc) {
            ++i;
            printf(".");
            sleep(30);
        }
        else {
            rc = DIDStore_StoreDID(store, doc, "");
            CU_ASSERT_NOT_EQUAL(rc, -1);
            createTxid = DIDDocument_GetTxid(doc);
            CU_ASSERT_STRING_EQUAL(txid, createTxid);
        }
    }
    free(txid);
    printf("\n-- resolve result: successfully!\n-- publish begin(update), wating...\n");

    DIDDocumentBuilder *builder = DIDDocument_Edit(doc);
    CU_ASSERT_PTR_NOT_NULL(builder);
    DIDDocument_Destroy(doc);

    DIDURL *credid = DIDURL_NewByDid(did, "cred-1");
    CU_ASSERT_PTR_NOT_NULL(credid);

    const char *types[] = {"BasicProfileCredential", "SelfClaimedCredential"};

    Property props[1];
    props[0].key = "name";
    props[0].value = "John";

    rc = DIDDocumentBuilder_AddSelfClaimedCredential(builder, credid, types, 2, props, 1, 0, storepass);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(doc);

    signkey = DIDDocument_GetDefaultPublicKey(doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    txid = (char *)DIDStore_PublishDID(store, storepass, did, signkey, true);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    printf("-- publish result:\n   did = %s\n   txid = %s\n-- resolve begin(update)", did->idstring, txid);

    while(!updatedoc || !strcmp(createTxid, updateTxid)) {
        if (updatedoc)
            DIDDocument_Destroy(updatedoc);

        updatedoc = DID_Resolve(did, true);
        if (!updatedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {
            rc = DIDStore_StoreDID(store, updatedoc, "");
            CU_ASSERT_NOT_EQUAL(rc, -1);
            updateTxid = DIDDocument_GetTxid(updatedoc);
            printf(".");
            continue;
        }
    }
    CU_ASSERT_STRING_EQUAL(txid, updateTxid);
    free(txid);
    printf("\n-- resolve result: successfully!\n------------------------------------------------------------\n");

    Mnemonic_Free((void*)mnemonic);
    DIDDocument_Destroy(doc);
    DIDDocument_Destroy(updatedoc);
    DIDDocument_Destroy(document);
}

static void test_idchain_update_nonexistedid(void)
{
    //todo: refer to java-testUpdateNonExistedDid
    return;
}

static void test_idchain_deactivedid_after_create(void)
{
    //todo: refer to java-testDeactivateSelfAfterCreate
    return;
}

static void test_idchain_deactivedid_after_update(void)
{
    //todo: refer to java-testDeactivateSelfAfterUpdate
    return;
}

static void test_idchain_deactivedid_with_authorization(void)
{
    //todo: refer to java-testDeactivateWithAuthorization1/
    //testDeactivateWithAuthorization2/testDeactivateWithAuthorization3
    return;
}

static void test_idchain_resolveall(void)
{
    return;
}

static int idchain_operation_test_suite_init(void)
{
    int rc;
    char _path[PATH_MAX];
    const char *storePath;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(storePath);
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
    { "test_idchain_publishdid",                     test_idchain_publishdid                      },
    { "test_idchain_publishdid_with_credential",     test_idchain_publishdid_with_credential      },
    { "test_idchain_update_nonexistedid",            test_idchain_update_nonexistedid             },
    { "test_idchain_deactivedid_after_create",       test_idchain_deactivedid_after_create        },
    { "test_idchain_deactivedid_after_update",       test_idchain_deactivedid_after_update        },
    { "test_idchain_deactivedid_with_authorization", test_idchain_deactivedid_with_authorization  },
    { "test_idchain_resolveall",                     test_idchain_resolveall                      },
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
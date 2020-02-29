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
    DIDDocument *doc = NULL, *updatedoc = NULL;
    const char *createTxid, *updateTxid;
    DID *did;
    int i = 0, rc;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    did = DIDDocument_GetSubject(document);
    CU_ASSERT_PTR_NOT_NULL(did);

    printf("\n-------------------------------------\n-- publish begin(create), waiting....\n");
    txid = (char *)DIDStore_PublishDID(store, storepass, did, signkey);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    printf("-- publish result:\n   did = %s\n   txid = %s\n", did->idstring, txid);
    free(txid);

    printf("-- resolve begin(create)");
    while(!doc) {
        doc = DID_Resolve(did);
        if (!doc) {
            ++i;
            printf(".");
            sleep(30);
        }
        else {
            rc = DIDStore_StoreDID(store, doc, "");
            CU_ASSERT_NOT_EQUAL(rc, -1);
            createTxid = DIDDocument_GetTxid(doc);
        }
    }
    printf("\n-- resolve result: successfully!\n-- publish begin(update), wating...\n");

    signkey = DIDDocument_GetDefaultPublicKey(doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    txid = (char *)DIDStore_PublishDID(store, storepass, did, signkey);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    printf("-- publish result:\n   did = %s\n   txid = %s\n", did->idstring, txid);
    free(txid);

    printf("-- resolve begin(update)");
    while(!updatedoc || !strcmp(createTxid, updateTxid)) {
        if (updatedoc)
            DIDDocument_Destroy(updatedoc);

        updatedoc = DID_Resolve(did);
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
    printf("\n-- resolve result: successfully!\n-------------------------------------\n");

    DIDDocument_Destroy(doc);
    DIDDocument_Destroy(updatedoc);
}

static void test_idchain_publishdid_with_credential(void)
{
    //todo
    return;
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

static int idchain_operation_test_suite_init(void)
{
    int rc;
    char _path[PATH_MAX];
    const char *storePath, *mnemonic;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(storePath);
    if (!store)
        return -1;

    mnemonic = Mnemonic_Generate(0);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", 0, true);
    Mnemonic_Free((char*)mnemonic);
    if (rc < 0) {
        TestData_Free();
        return -1;
    }

    document = DIDStore_NewDID(store, storepass, "littlefish");
    if (!document) {
        TestData_Free();
        return -1;
    }

    return 0;
}

static int idchain_operation_test_suite_cleanup(void)
{
    DIDDocument_Destroy(document);
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
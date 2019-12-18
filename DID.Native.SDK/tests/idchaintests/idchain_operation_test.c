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

static DIDDocument *document;

static void test_idchain_publishdid(void)
{
    int rc;
    DIDURL *signkey;
    DIDStore *store;
    DIDDocument *doc = NULL;
    DID *did;
    int i = 0;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    store = DIDStore_GetInstance();
    rc = DIDStore_PublishDID(store, document, signkey, storepass);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    did = DIDDocument_GetSubject(document);
    CU_ASSERT_PTR_NOT_NULL(did);

    printf("\n#### begin to resolve [%s]........", did->idstring);
    while(!doc) {
        doc = DIDStore_ResolveDID(store, did, true);
        if (!doc) {
            printf("#### no document resolved: time = %d.\n", ++i);
            sleep(30);
        }
        else {
            printf("\n##### document resolved: time = %d.\n", ++i);
            printf("\n##### did: %s\n", did->idstring);
            printf("\n##### document: %s\n", DIDDocument_ToJson(doc, 0, 0));
        }

    }
    printf("\n#### end resolve\n");
}

static void test_idchain_updatedid(void)
{
    int rc;
    DIDURL *signkey;
    DIDStore *store;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    store = DIDStore_GetInstance();
    rc = DIDStore_UpdateDID(store, document, signkey, storepass);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_idchain_deactivatedid(void)
{
    int rc;
    DIDURL *signkey;
    DIDStore *store;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    store = DIDStore_GetInstance();
    rc = DIDStore_DeactivateDID(store, DIDDocument_GetSubject(document), signkey, storepass);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static int idchain_operation_test_suite_init(void)
{
    int rc;
    char _path[PATH_MAX];
    const char *storePath, *mnemonic;
    DIDStore *store;

    storePath = get_store_path(_path, "/idchain");
    rc = TestData_SetupStore(storePath);
    if (rc < 0)
        return -1;

    store = DIDStore_GetInstance();
    mnemonic = Mnemonic_Generate(0);
    printf("\n#### mnemonic: %s\n", mnemonic);
    rc = DIDStore_InitPrivateIdentity(store, mnemonic, "", storepass, 0, true);
    Mnemonic_free((char*)mnemonic);
    if (rc < 0)
        return -1;

    document = DIDStore_NewDID(store, storepass, "littlefish");
    if (!document)
        return -1;

    return 0;
}

static int idchain_operation_test_suite_cleanup(void)
{
    TestData_Free();
    DIDDocument_Destroy(document);
    DIDStore_Deinitialize();
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_idchain_publishdid",           test_idchain_publishdid      },
    //{   "test_didrequest_updatedid",         test_didrequest_updatedid       },
    //{   "test_didrequest_deactivatedid",     test_didrequest_deactivatedid   },
    {   NULL,                                 NULL                           }
};

static CU_SuiteInfo suite[] = {
    { "id chain operateion test", idchain_operation_test_suite_init, idchain_operation_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                      NULL,                              NULL,                                 NULL, NULL, NULL  }
};

CU_SuiteInfo* idchain_operation_test_suite_info(void)
{
    return suite;
}
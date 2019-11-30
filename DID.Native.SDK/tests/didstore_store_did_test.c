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
#include "didtest_adapter.h"
#include "ela_did.h"
#include "did.h"
#include "diddocument.h"
#include "didstore.h"

#define  TEST_LEN    512

static DIDDocument *document;
static DID *did;
static DIDAdapter *adapter;

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return storepass;
}

static void test_didstore_store_did(void)
{
    int rc;
    DIDStore *store;

    store = DIDStore_GetInstance();

    rc = DIDStore_StoreDID(store, document, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, document, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, document, "littlefish");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDStore_DeleteDID(store, did);
}

static int didstore_store_test_suite_init(void)
{
    char _path[PATH_MAX], _dir[TEST_LEN];
    char *storePath, *walletDir;
    DIDStore *store;

    walletDir = get_wallet_path(_dir, "/.didwallet");
    adapter = TestAdapter_Create(walletDir, walletId, network, resolver, getpassword);
    if (!adapter)
        return -1;

    storePath = get_store_path(_path, "/servet");
    store = DIDStore_Initialize(storePath, adapter);
    if (!store) {
        TestAdapter_Destroy(adapter);
        return -1;
    }

    document = DIDDocument_FromJson(global_did_string);
    if(!document) {
        TestAdapter_Destroy(adapter);
        DIDStore_Deinitialize(store);
        return -1;
    }

    did = DIDDocument_GetSubject(document);
    if (!did) {
        DIDDocument_Destroy(document);
        TestAdapter_Destroy(adapter);
        DIDStore_Deinitialize(store);
        return -1;
    }

    return 0;
}

static int didstore_store_test_suite_cleanup(void)
{
    DIDStore *store;

    TestAdapter_Destroy(adapter);
    DIDDocument_Destroy(document);

    store = DIDStore_GetInstance();
    DIDStore_Deinitialize(store);
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_store_did",             test_didstore_store_did     },
    {   NULL,                                  NULL                        }
};

static CU_SuiteInfo suite[] = {
    {   "didstore did test",    didstore_store_test_suite_init,    didstore_store_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                  NULL,                         NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_store_did_test_suite_info(void)
{
    return suite;
}
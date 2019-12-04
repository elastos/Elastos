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
//#include "did.h"
//#include "didstore.h"
//#include "diddocument.h"

#define  TEST_LEN    512

static DIDAdapter *adapter;

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return storepass;
}

static void test_didstore_new_did(void)
{
    DID *did;
    int rc;
    DIDStore *store;

    store = DIDStore_GetInstance();

    rc = DIDStore_InitPrivateIdentity(store, mnemonic, "", "", 0, false);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    DIDDocument *document = DIDStore_NewDID(store, storepass, "littlefish");
    CU_ASSERT_PTR_NOT_NULL(document);

    did = DIDDocument_GetSubject(document);
    DIDStore_DeleteDID(store, did);
    DIDDocument_Destroy(document);
}

static int didstore_new_test_suite_init(void)
{
    char _path[PATH_MAX], _dir[TEST_LEN];
    char *storePath, *walletDir;
    DIDStore *store;

    walletDir = get_wallet_path(_dir, "/.didwallet");
    adapter = TestAdapter_Create(walletDir, walletId, network, resolver, getpassword);
    if (!adapter)
        return -1;

    storePath = get_store_path(_path, "/newdid");
    store = DIDStore_Initialize(storePath, adapter);
    if (!store) {
        TestAdapter_Destroy(adapter);
        return -1;
    }

    return 0;
}

static int didstore_new_test_suite_cleanup(void)
{
    DIDStore_Deinitialize();
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_new_did",        test_didstore_new_did     },
    {   NULL,                                  NULL                        }
};

static CU_SuiteInfo suite[] = {
    {   "didstore new did test",    didstore_new_test_suite_init,    didstore_new_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                  NULL,                         NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_new_did_test_suite_info(void)
{
    return suite;
}
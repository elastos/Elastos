#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <CUnit/Basic.h>
#include <crystal.h>
#include <limits.h>

#include "constant.h"
#include "loader.h"
#include "ela_did.h"
#include "didstore.h"
#include "didtest_adapter.h"

#define  TEST_LEN    512

static DIDAdapter *adapter;

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return storepass;
}

static void test_didstore_initial(void)
{
    char _path[PATH_MAX];
    char *storePath;
    DIDStore *store;

    storePath = get_store_path(_path, "/servet");
    store = DIDStore_Initialize(storePath, adapter);

    CU_ASSERT_PTR_NOT_NULL(store);
    DIDStore_Deinitialize(store);
}

static int didstore_initial_test_suite_init(void)
{
    char _dir[TEST_LEN];
    char *walletDir;

    walletDir = get_wallet_path(_dir, "/.didwallet");
    adapter = TestAdapter_Create(walletDir, walletId, network, resolver, getpassword);
    if (!adapter)
        return -1;

    return  0;
}

static int didstore_initial_test_suite_cleanup(void)
{
    TestAdapter_Destroy(adapter);
    return 0;
}

static CU_TestInfo cases[] = {
    {  "test_didstore_initial",   test_didstore_initial     },
    {  NULL,                      NULL                      }
};

static CU_SuiteInfo suite[] = {
    {  "didstore initial test",  didstore_initial_test_suite_init,  didstore_initial_test_suite_cleanup,   NULL, NULL, cases },
    {  NULL,                     NULL,                              NULL,                                  NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_initial_test_suite_info(void)
{
    return suite;
}
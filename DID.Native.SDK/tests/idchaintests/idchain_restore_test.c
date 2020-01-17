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

static int didcount = 0;

static int get_did(DID *did, void *context)
{
    char alias[ELA_MAX_ALIAS_LEN];
    DIDDocument *doc = NULL;
    int rc;

    if (!did)
        return 0;

    didcount++;
    return 0;
}

static void test_idchain_restore(void)
{
    int rc;
    char _path[PATH_MAX];
    const char *storePath, *mnemonic;
    DIDStore *store;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    rc = DIDStore_InitPrivateIdentity(store, TestData_LoadRestoreMnemonic(),
        "secret", storepass, 0, true);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    printf("Synchronizing from IDChain...");
    DIDStore_Synchronize(store, storepass);
    printf("OK!\n");

    rc = DIDStore_ListDID(store, get_did, NULL);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(didcount, 5);

    //todo: more operation(refer to java)
    TestData_Free();
}

static int idchain_restore_test_suite_init(void)
{
    return 0;
}

static int idchain_restore_test_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_idchain_restore",      test_idchain_restore     },
    {   NULL,                        NULL                     }
};

static CU_SuiteInfo suite[] = {
    { "id chain restore test", idchain_restore_test_suite_init, idchain_restore_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                   NULL,                            NULL,                               NULL, NULL, NULL  }
};

CU_SuiteInfo* idchain_restore_test_suite_info(void)
{
    return suite;
}
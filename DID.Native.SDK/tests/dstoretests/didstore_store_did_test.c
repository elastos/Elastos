#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <crystal.h>
#include <CUnit/Basic.h>
#include <limits.h>

#include "ela_did.h"
#include "constant.h"
#include "loader.h"
#include "did.h"

static DIDDocument *document;
static DIDStore *store;
static const char *storePath;

static void test_didstore_store_did(void)
{
    const char *path;
    char _path[PATH_MAX];
    DID *did;
    int rc;

    did = DIDDocument_GetSubject(document);
    CU_ASSERT_PTR_NOT_NULL_FATAL(did);

    rc = DIDStore_StoreDID(store, document, "littlefish");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    path = get_file_path(_path, PATH_MAX, 5, storePath, storedirroot, "/",
            did->idstring, "/document");
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 7, storePath, storedirroot, "/",
            did->idstring, "/credentials", "/email", "/credential");
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 7, storePath, storedirroot, "/",
            did->idstring, "/credentials", "/profile", "/credential");
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    rc = DIDStore_StoreDID(store, document, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(store, document, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static int didstore_storedid_test_suite_init(void)
{
    char _path[PATH_MAX];
    int rc;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(storePath);
    if (!store)
        return -1;

    document = DIDDocument_FromJson(TestData_LoadDocJson());
    if(!document) {
        TestData_Free();
        return -1;
    }

    return 0;
}

static int didstore_storedid_test_suite_cleanup(void)
{
    DIDDocument_Destroy(document);
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_store_did",             test_didstore_store_did     },
    {   NULL,                                  NULL                        }
};

static CU_SuiteInfo suite[] = {
    { "didstore did test", didstore_storedid_test_suite_init, didstore_storedid_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,               NULL,                              NULL,                                 NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_store_did_test_suite_info(void)
{
    return suite;
}
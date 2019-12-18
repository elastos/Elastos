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

static DIDDocument *document;
static DID *did;

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

static int didstore_storedid_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    DIDStore *store;
    int rc;

    storePath = get_store_path(_path, "/servet");
    rc = TestData_SetupStore(storePath);
    if (rc < 0)
        return -1;

    document = DIDDocument_FromJson(TestData_LoadDocJson());
    if(!document) {
        DIDStore_Deinitialize();
        return -1;
    }

    did = DIDDocument_GetSubject(document);
    if (!did) {
        DIDDocument_Destroy(document);
        DIDStore_Deinitialize();
        return -1;
    }

    return 0;
}

static int didstore_storedid_test_suite_cleanup(void)
{
    DIDDocument_Destroy(document);
    DIDStore_Deinitialize();
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
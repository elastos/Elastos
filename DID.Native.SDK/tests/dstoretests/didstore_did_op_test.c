#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <crystal.h>
#include <CUnit/Basic.h>
#include <limits.h>

#include "loader.h"
#include "constant.h"
#include "ela_did.h"
#include "did.h"
#include "didstore.h"

static DIDDocument *document;
static DID *did;
static DIDStore *store;

int get_did(DID *did, void *context)
{
    char _did[ELA_MAX_DID_LEN];

    if(!did)
        return 0;

    printf("\n did: %s\n", DID_ToString(did, _did, sizeof(_did)));
    return 0;
}

static void test_didstore_contain_did(void)
{
    bool rc;

    rc = DIDStore_ContainsDID(store, did);
    CU_ASSERT_NOT_EQUAL(rc, false);
}

static void test_didstore_list_did(void)
{
    int rc;

    rc = DIDStore_ListDID(store, get_did, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didstore_load_did(void)
{
    DIDDocument *doc;

    doc = DIDStore_LoadDID(store, did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocument_Destroy(doc);
}

static void test_didstore_delete_did(void)
{
    if(DIDStore_ContainsDID(store, did))
        DIDStore_DeleteDID(store, did);

    bool rc = DIDStore_ContainsDID(store, did);
    CU_ASSERT_NOT_EQUAL(rc, true);
}

static int didstore_did_op_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
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

    did = DIDDocument_GetSubject(document);
    if (!did) {
        DIDDocument_Destroy(document);
        TestData_Free();
        return -1;
    }

    return DIDStore_StoreDID(store, document, "littlefish");
}

static int didstore_did_op_test_suite_cleanup(void)
{
    DIDDocument_Destroy(document);
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_contain_did",       test_didstore_contain_did     },
    {   "test_didstore_list_did",          test_didstore_list_did        },
    {   "test_didstore_load_did",          test_didstore_load_did        },
    {   "test_didstore_delete_did",        test_didstore_delete_did      },
    {   NULL,                              NULL                          }
};

static CU_SuiteInfo suite[] = {
    {   "didstore did op test",    didstore_did_op_test_suite_init,    didstore_did_op_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                  NULL,                         NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_did_op_test_suite_info(void)
{
    return suite;
}
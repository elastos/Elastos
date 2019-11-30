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
#include "didtest_adapter.h"
#include "ela_did.h"
#include "did.h"
#include "didstore.h"
#include "diddocument.h"

#define  TEST_LEN    512

static DIDDocument *document;
static DID *did;
static DIDAdapter *adapter;

int get_did_hint(DIDEntry *entry, void *context)
{
    if(!entry)
        return -1;

    printf("\n did: %s, hint: %s\n", entry->did.idstring, entry->hint);
    free(entry);
    return 0;
}

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return storepass;
}

static void test_didstore_contain_did(void)
{
    bool rc;
    DIDStore *store;

    store = DIDStore_GetInstance();
    rc = DIDStore_ContainsDID(store, did);
    CU_ASSERT_NOT_EQUAL(rc, false);
}

static void test_didstore_list_did(void)
{
    int rc;
    DIDStore *store;

    store = DIDStore_GetInstance();
    rc = DIDStore_ListDID(store, get_did_hint, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didstore_load_did(void)
{
    DIDDocument *doc;
    DIDStore *store;

    store = DIDStore_GetInstance();
    doc = DIDStore_LoadDID(store, did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocument_Destroy(doc);
}

static void test_didstore_delete_did(void)
{
    DIDStore *store;

    store = DIDStore_GetInstance();
    if(DIDStore_ContainsDID(store, did) == true) {
        DIDStore_DeleteDID(store, did);
    }

    bool rc = DIDStore_ContainsDID(store, did);
    CU_ASSERT_NOT_EQUAL(rc, true);
}

static int didstore_did_op_test_suite_init(void)
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

    return DIDStore_StoreDID(store, document, "littlefish");
}

static int didstore_did_op_test_suite_cleanup(void)
{
    DIDStore *store;

    TestAdapter_Destroy(adapter);
    DIDDocument_Destroy(document);

    store = DIDStore_GetInstance();
    DIDStore_DeleteDID(store, did);
    DIDStore_Deinitialize(store);
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
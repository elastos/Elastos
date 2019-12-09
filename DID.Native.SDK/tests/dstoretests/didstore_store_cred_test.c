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
#include "didtest_adapter.h"
#include "did.h"
#include "credential.h"

#define  TEST_LEN    512

static DID did;
static Credential *credential;
static DIDAdapter *adapter;

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return storepass;
}

static void test_didstore_store_cred(void)
{
    int rc;
    DIDStore *store;

    store = DIDStore_GetInstance();
    rc = DIDStore_StoreCredential(store, credential, "me");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDStore_DeleteCredential(store, &did, &(credential->id));
}

static int didstore_storecred_test_suite_init(void)
{
    char _path[PATH_MAX], _dir[TEST_LEN];
    char *storePath, *walletDir;
    DIDStore *store;
    DIDDocument *doc;

    walletDir = get_wallet_path(_dir, "/.didwallet");
    adapter = TestDIDAdapter_Create(walletDir, walletId, network, resolver, getpassword);
    if (!adapter)
        return -1;

    storePath = get_store_path(_path, "/servet");
    store = DIDStore_Initialize(storePath, adapter);
    if (!store)
        return -1;

    doc = DIDDocument_FromJson(global_did_string);
    if(!doc) {
        TestDIDAdapter_Destroy(adapter);
        return -1;
    }

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    credential = Credential_FromJson(global_cred_string, &did);
    if(!credential) {
        TestDIDAdapter_Destroy(adapter);
        return -1;
    }

    return 0;
}

static int didstore_storecred_test_suite_cleanup(void)
{
    TestDIDAdapter_Destroy(adapter);
    Credential_Destroy(credential);
    DIDStore_Deinitialize();
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_store_cred",            test_didstore_store_cred     },
    {   NULL,                                  NULL                        }
};

static CU_SuiteInfo suite[] = {
    {   "didstore did test",    didstore_storecred_test_suite_init,    didstore_storecred_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                  NULL,                                  NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_store_cred_test_suite_info(void)
{
    return suite;
}
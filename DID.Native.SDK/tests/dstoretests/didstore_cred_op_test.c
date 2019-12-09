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
#include "credential.h"
#include "did.h"
#include "didstore.h"

#define  TEST_LEN    512

static DID did;
static Credential *credential;
static DIDAdapter *adapter;

int get_cred_hint(CredentialEntry *entry, void *context)
{
    if(!entry)
        return -1;

    printf("\n credential: %s#%s, hint: %s\n",
            entry->id.did.idstring, entry->id.fragment, entry->hint);
    free(entry);
    return 0;
}

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return storepass;
}

static void test_didstore_contain_creds(void)
{
    bool rc;
    DIDStore *store;

    store = DIDStore_GetInstance();

    rc = DIDStore_ContainsCredentials(store, &did);
    CU_ASSERT_NOT_EQUAL(rc, false);
    rc = DIDStore_ContainsCredential(store, &did, &(credential->id));
    CU_ASSERT_NOT_EQUAL(rc, false);
}

static void test_didstore_list_cred(void)
{
    DIDStore *store;

    store = DIDStore_GetInstance();

    int rc = DIDStore_ListCredentials(store, &did, get_cred_hint, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didstore_select_cred(void)
{
    DIDStore *store;

    store = DIDStore_GetInstance();

    int rc = DIDStore_SelectCredentials(store, &did, &(credential->id),
            "BasicProfileCredential", get_cred_hint, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didstore_load_cred(void)
{
    DIDStore *store;
    Credential *cred;

    store = DIDStore_GetInstance();

    cred = DIDStore_LoadCredential(store, &did, &(credential->id));
    CU_ASSERT_PTR_NOT_NULL(cred);

    Credential_Destroy(cred);
}

static void test_didstore_delete_cred(void)
{
    DIDStore *store;

    store = DIDStore_GetInstance();
    if(DIDStore_ContainsCredential(store, &did, &(credential->id)) == true) {
        DIDStore_DeleteCredential(store, &did, &(credential->id));
    }

    bool rc = DIDStore_ContainsCredential(store, &did, &(credential->id));
    CU_ASSERT_NOT_EQUAL(rc, true);
}

static int didstore_cred_op_test_suite_init(void)
{
    char _path[PATH_MAX], _dir[TEST_LEN];
    char *storePath, *walletDir;
    DIDStore *store;

    walletDir = get_wallet_path(_dir, "/.didwallet");
    adapter = TestDIDAdapter_Create(walletDir, walletId, network, resolver, getpassword);
    if (!adapter)
        return -1;

    storePath = get_store_path(_path, "/servet");
    store = DIDStore_Initialize(storePath, adapter);
    if (!store) {
        TestDIDAdapter_Destroy(adapter);
        return -1;
    }

    DIDDocument *doc = DIDDocument_FromJson(global_did_string);
    if(!doc) {
        DIDStore_Deinitialize();
        TestDIDAdapter_Destroy(adapter);
        return -1;
    }

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    credential = Credential_FromJson(global_cred_string, &did);
    if(!credential) {
        DIDStore_Deinitialize();
        TestDIDAdapter_Destroy(adapter);
        return -1;
    }

    return DIDStore_StoreCredential(store, credential, "me");
}

static int didstore_cred_op_test_suite_cleanup(void)
{
    DIDStore *store = DIDStore_GetInstance();

    TestDIDAdapter_Destroy(adapter);
    Credential_Destroy(credential);
    DIDStore_DeleteDID(store, &did);
    DIDStore_Deinitialize();
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_contain_creds",      test_didstore_contain_creds     },
    {   "test_didstore_list_cred",          test_didstore_list_cred         },
    {   "test_didstore_select_cred",        test_didstore_select_cred       },
    {   "test_didstore_load_cred",          test_didstore_load_cred         },
    {   "test_didstore_delete_cred",        test_didstore_delete_cred       },
    {   NULL,                               NULL                            }
};

static CU_SuiteInfo suite[] = {
    {   "didstore cred op test",    didstore_cred_op_test_suite_init,    didstore_cred_op_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                     NULL,                         NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_cred_op_test_suite_info(void)
{
    return suite;
}
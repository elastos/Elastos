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
#include "credential.h"
#include "did.h"
#include "didstore.h"

static DID did;
static Credential *credential;
static DIDStore *store;

int get_cred_alias(CredentialEntry *entry, void *context)
{
    if(!entry)
        return -1;

    printf("\n credential: %s#%s, alias: %s\n",
            entry->id.did.idstring, entry->id.fragment, entry->alias);
    return 0;
}

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return walletpass;
}

static void test_didstore_contain_creds(void)
{
    bool rc;

    rc = DIDStore_ContainsCredentials(store, &did);
    CU_ASSERT_TRUE(rc);
    rc = DIDStore_ContainsCredential(store, &did, &(credential->id));
    CU_ASSERT_TRUE(rc);
}

static void test_didstore_list_cred(void)
{
    int rc = DIDStore_ListCredentials(store, &did, get_cred_alias, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didstore_select_cred(void)
{
    int rc = DIDStore_SelectCredentials(store, &did, &(credential->id),
            "BasicProfileCredential", get_cred_alias, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didstore_load_cred(void)
{
    Credential *cred;

    cred = DIDStore_LoadCredential(store, &did, &(credential->id));
    CU_ASSERT_PTR_NOT_NULL(cred);

    Credential_Destroy(cred);
}

static void test_didstore_delete_cred(void)
{
    if(DIDStore_ContainsCredential(store, &did, &(credential->id)) == true) {
        DIDStore_DeleteCredential(store, &did, &(credential->id));
    }

    bool rc = DIDStore_ContainsCredential(store, &did, &(credential->id));
    CU_ASSERT_NOT_EQUAL(rc, true);
}

static int didstore_cred_op_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    int rc;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(storePath);
    if (!store)
        return -1;

    DIDDocument *doc = DIDDocument_FromJson(TestData_LoadDocJson());
    if(!doc) {
        TestData_Free();
        return -1;
    }

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    credential = Credential_FromJson(TestData_LoadVcEmailJson(), &did);
    if(!credential) {
        TestData_Free();
        return -1;
    }

    return DIDStore_StoreCredential(store, credential, "me");
}

static int didstore_cred_op_test_suite_cleanup(void)
{
    Credential_Destroy(credential);
    DIDStore_DeleteDID(store, &did);
    TestData_Free();
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
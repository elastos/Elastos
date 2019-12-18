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
#include "credential.h"

#define  TEST_LEN    512

static DID did;
static Credential *credential;

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
    char _path[PATH_MAX];
    const char *storePath;
    DIDStore *store;
    DIDDocument *doc;
    int rc;

    storePath = get_store_path(_path, "/servet");
    rc = TestData_SetupStore(storePath);
    if (rc < 0)
        return -1;

    doc = DIDDocument_FromJson(TestData_LoadDocJson());
    if(!doc)
        return -1;

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    credential = Credential_FromJson(TestData_LoadVcEmailJson(), &did);
    if(!credential)
        return -1;

    return 0;
}

static int didstore_storecred_test_suite_cleanup(void)
{
    TestData_Free();
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
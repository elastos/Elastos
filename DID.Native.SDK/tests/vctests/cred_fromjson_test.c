#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <limits.h>

#include <CUnit/Basic.h>
#include "ela_did.h"
#include "loader.h"

static DIDDocument *doc;
static DID *did;

static void test_cred_fromjson(void)
{
    Credential *credential = TestData_LoadEmailVc();
    CU_ASSERT_PTR_NOT_NULL_FATAL(credential);
}

static int cred_fromjson_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    DIDStore *store;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(storePath);
    if (!store)
        return -1;

    doc = DIDDocument_FromJson(TestData_LoadDocJson());
    if(!doc)
        return -1;

    did = DIDDocument_GetSubject(doc);

    return 0;
}

static int cred_fromjson_test_suite_cleanup(void)
{
    TestData_Free();
    DIDDocument_Destroy(doc);
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_cred_fromjson",            test_cred_fromjson    },
    { NULL,                            NULL                  }
};

static CU_SuiteInfo suite[] = {
    { "credential from json test",   cred_fromjson_test_suite_init,   cred_fromjson_test_suite_cleanup,     NULL, NULL, cases },
    {  NULL,                         NULL,                            NULL,                                 NULL, NULL, NULL  }
};


CU_SuiteInfo* cred_fromjson_test_suite_info(void)
{
    return suite;
}

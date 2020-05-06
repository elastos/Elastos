#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <limits.h>

#include <CUnit/Basic.h>
#include "constant.h"
#include "loader.h"
#include "ela_did.h"
#include "did.h"

static DIDDocument *document;
static DIDStore *store;

static void test_diddoc_json_operateion(void)
{
    DIDDocument *compactdoc, *normalizedoc, *doc;

    compactdoc = DIDDocument_FromJson(TestData_LoadDocCompJson());
    CU_ASSERT_PTR_NOT_NULL(compactdoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(compactdoc));

    CU_ASSERT_EQUAL(4, DIDDocument_GetPublicKeyCount(compactdoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(compactdoc));
    CU_ASSERT_EQUAL(1, DIDDocument_GetAuthorizationCount(compactdoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetCredentialCount(compactdoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetServiceCount(compactdoc));

    normalizedoc = DIDDocument_FromJson(TestData_LoadDocNormJson());
    CU_ASSERT_PTR_NOT_NULL(normalizedoc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(normalizedoc));

    CU_ASSERT_EQUAL(4, DIDDocument_GetPublicKeyCount(normalizedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetAuthenticationCount(normalizedoc));
    CU_ASSERT_EQUAL(1, DIDDocument_GetAuthorizationCount(normalizedoc));
    CU_ASSERT_EQUAL(2, DIDDocument_GetCredentialCount(normalizedoc));
    CU_ASSERT_EQUAL(3, DIDDocument_GetServiceCount(normalizedoc));

    doc = TestData_LoadDoc();
    CU_ASSERT_PTR_NOT_NULL(doc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(doc));

    CU_ASSERT_STRING_EQUAL(TestData_LoadDocNormJson(), DIDDocument_ToJson(compactdoc, true));
    CU_ASSERT_STRING_EQUAL(TestData_LoadDocNormJson(), DIDDocument_ToJson(normalizedoc, true));
    CU_ASSERT_STRING_EQUAL(TestData_LoadDocNormJson(), DIDDocument_ToJson(doc, true));

    CU_ASSERT_STRING_EQUAL(TestData_LoadDocCompJson(), DIDDocument_ToJson(compactdoc, false));
    CU_ASSERT_STRING_EQUAL(TestData_LoadDocCompJson(), DIDDocument_ToJson(normalizedoc, false));
    CU_ASSERT_STRING_EQUAL(TestData_LoadDocCompJson(), DIDDocument_ToJson(doc, false));
}

static int diddoc_json_op_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    int rc;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(false, storePath);
    if (!store)
        return -1;

    rc = TestData_InitIdentity(store);
    if (rc) {
        TestData_Free();
        return -1;
    }

    return 0;
}

static int diddoc_json_op_test_suite_cleanup(void)
{
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_diddoc_json_operateion",   test_diddoc_json_operateion   },
    { NULL,                            NULL                          }
};

static CU_SuiteInfo suite[] = {
    { "diddoc json operation test",  diddoc_json_op_test_suite_init,  diddoc_json_op_test_suite_cleanup, NULL, NULL, cases },
    { NULL,                          NULL,                            NULL,                              NULL, NULL, NULL  }
};


CU_SuiteInfo* diddoc_json_op_test_suite_info(void)
{
    return suite;
}

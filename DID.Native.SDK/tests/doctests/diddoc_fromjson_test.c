#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include "ela_did.h"
#include "loader.h"

static void test_diddoc_from_json(void)
{
    DIDDocument *document;
    const char *data;

    document = DIDDocument_FromJson(TestData_LoadDocJson());
    CU_ASSERT_PTR_NOT_NULL_FATAL(document);

    data = DIDDocument_ToJson(document, 0, 0);
    CU_ASSERT_PTR_NOT_NULL_FATAL(data);
    printf("\n#### get document: %s\n", data);

    free(data);
    DIDDocument_Destroy(document);
    TestData_Free();
}

static int diddoc_test_fromjson_suite_init(void)
{
    return  0;
}

static int diddoc_test_fromjson_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_diddoc_from_json",       test_diddoc_from_json    },
    {   NULL,                          NULL                     }
};

static CU_SuiteInfo suite[] = {
    {   "diddoc from json test",    diddoc_test_fromjson_suite_init,     diddoc_test_fromjson_suite_cleanup,    NULL, NULL, cases },
    {    NULL,                      NULL,                                NULL,                                  NULL, NULL, NULL  }
};

CU_SuiteInfo* diddoc_fromjson_test_suite_info(void)
{
    return suite;
}
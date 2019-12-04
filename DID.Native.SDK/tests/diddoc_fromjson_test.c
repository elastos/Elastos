#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include "ela_did.h"
//#include "diddocument.h"
#include "loader.h"

#define  TEST_LEN    512

static DIDDocument *document;

static void test_diddoc_from_json(void)
{
    document = DIDDocument_FromJson(global_did_string);
    CU_ASSERT_PTR_NOT_NULL_FATAL(document);
    DIDDocument_Destroy(document);
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
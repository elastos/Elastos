#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include "ela_did.h"
#include "did.h"
#include "constant.h"

static void test_didurl_fromString(void)
{
    DID *did = DID_FromString(testdid_string);

    DIDURL *id = DIDURL_FromString(testid_string, did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    DIDURL_Destroy(id);

    id = DIDURL_FromString("#default", did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    DIDURL_Destroy(id);
}

static void test_didurl_fromString_error(void)
{
    DIDURL *id = DIDURL_FromString(testid_string, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    DIDURL_Destroy(id);

    id = DIDURL_FromString(compact_idstring, NULL);
    CU_ASSERT_PTR_NULL(id);
    DIDURL_Destroy(id);
}

static void test_didurl_new_didurl(void)
{
    char _idstring[ELA_MAX_DIDURL_LEN];
    const char *idstring;

    DIDURL *id = DIDURL_New(method_specific_string, fragment);
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);

    idstring = DIDURL_ToString(id, _idstring, sizeof(_idstring), false);
    DIDURL_Destroy(id);
    CU_ASSERT_PTR_NOT_NULL_FATAL(idstring);
    CU_ASSERT_STRING_EQUAL(idstring, testid_string);
}

static void test_didurl_new_didurl_error(void)
{
    DIDURL *id = DIDURL_New("", fragment);
    CU_ASSERT_PTR_NULL(id);

    id = DIDURL_New(method_specific_string, "");
    CU_ASSERT_PTR_NULL(id);
}

static int didurl_test_constructor_suite_init(void)
{
    return 0;
}

static int didurl_test_constructor_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {  "test_didurl_fromString",                test_didurl_fromString         },
    {  "test_didurl_fromString_error",          test_didurl_fromString_error   },
    {  "test_didurl_new_didurl",                test_didurl_new_didurl         },
    {  "test_didurl_new_didurl_error",          test_didurl_new_didurl_error   },
    {   NULL,                                   NULL                           }
};

static CU_SuiteInfo suite[] = {
    { "didurl constructor test", didurl_test_constructor_suite_init, didurl_test_constructor_suite_cleanup, NULL, NULL, cases },
    {  NULL,                     NULL,                               NULL,                                  NULL, NULL, NULL  }
};

CU_SuiteInfo* didurl_constructor_test_suite_info(void)
{
    return suite;
}
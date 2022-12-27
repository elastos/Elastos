#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include "ela_did.h"
#include "did.h"
#include "constant.h"

#define  TEST_LEN    512

static void test_did_fromString(void)
{
    DID *did = DID_FromString(testdid_string);
    CU_ASSERT_PTR_NOT_NULL_FATAL(did);
    DID_Destroy(did);
}

static void test_did_fromString_error(void)
{
    DID *did = DID_FromString("did:example:iYpQMwheDxySqivocSJaoprcoDTqQsDYAu");
    CU_ASSERT_PTR_NULL(did);
    DID_Destroy(did);

    did = DID_FromString("did:elastos:");
    CU_ASSERT_PTR_NULL(did);
    DID_Destroy(did);
}

static void test_did_new_did(void)
{
    char _didstring[ELA_MAX_DID_LEN];
    const char *didstring;

    DID *did = DID_New(method_specific_string);
    CU_ASSERT_PTR_NOT_NULL_FATAL(did);

    didstring = DID_ToString(did, _didstring, sizeof(_didstring));
    DID_Destroy(did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(didstring);
    CU_ASSERT_STRING_EQUAL(didstring, testdid_string);
}

static void test_did_new_did_error(void)
{
    DID *did = DID_New("");
    CU_ASSERT_PTR_NULL(did);
}

static int did_test_constructor_suite_init(void)
{
    return  0;
}

static int did_test_constructor_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {  "test_did_fromString",             test_did_fromString         },
    {  "test_did_fromString_error",       test_did_fromString_error   },
    {  "test_did_new_did",                test_did_new_did            },
    {  "test_did_new_did_error",          test_did_new_did_error      },
    {   NULL,                             NULL                        }
};

static CU_SuiteInfo suite[] = {
    { "did constructor test", did_test_constructor_suite_init, did_test_constructor_suite_cleanup, NULL, NULL, cases },
    {  NULL,                  NULL,                            NULL,                               NULL, NULL, NULL  }
};

CU_SuiteInfo* did_constructor_test_suite_info(void)
{
    return suite;
}
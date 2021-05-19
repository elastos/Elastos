#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include "ela_did.h"
#include "did.h"
#include "constant.h"

static DID *did;

static void test_did_get_method(void)
{
    CU_ASSERT_STRING_EQUAL(DID_GetMethod(did), "elastos");
}

static void test_did_get_specificid(void)
{
    CU_ASSERT_STRING_EQUAL(DID_GetMethodSpecificId(did), method_specific_string);
}

static void test_did_tostring(void)
{
    char id[ELA_MAX_DID_LEN];
    CU_ASSERT_STRING_EQUAL(DID_ToString(did, id, sizeof(id)), testdid_string);
}

static void test_did_tostring_error(void)
{
    char id[20];
    CU_ASSERT_PTR_NULL(DID_ToString(did, id, sizeof(id)));
}

static void test_did_compare(void)
{
    int rc;

    DID *comdid = DID_New("abc");
    rc = DID_Compare(comdid, did);
    DID_Destroy(comdid);
    CU_ASSERT_TRUE(rc < 0);

    comdid = DID_New("zyx");
    rc = DID_Compare(comdid, did);
    DID_Destroy(comdid);
    CU_ASSERT_TRUE(rc > 0);

    comdid = DID_New(method_specific_string);
    rc = DID_Compare(comdid, did);
    DID_Destroy(comdid);
    CU_ASSERT_TRUE(rc == 0);
}

static void test_did_equals(void)
{
    bool isEqual;

    DID *equaldid = DID_New(method_specific_string);
    isEqual = DID_Equals(equaldid, did);
    DID_Destroy(equaldid);
    CU_ASSERT_TRUE(isEqual);

    equaldid = DID_New("abc");
    isEqual = DID_Equals(equaldid, did);
    DID_Destroy(equaldid);
    CU_ASSERT_FALSE(isEqual);
}

static int did_test_operation_suite_init(void)
{
    did = DID_FromString(testdid_string);
    if(!did)
        return -1;

    return  0;
}

static int did_test_operation_suite_cleanup(void)
{
    DID_Destroy(did);
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_did_get_method",                 test_did_get_method      },
    {   "test_did_get_specificid",             test_did_get_specificid  },
    {   "test_did_tostring",                   test_did_tostring        },
    {   "test_did_tostring_error",             test_did_tostring_error  },
    {   "test_did_compare",                    test_did_compare         },
    {   "test_did_equals",                     test_did_equals          },
    {   NULL,                                  NULL                     }
};

static CU_SuiteInfo suite[] = {
    { "did operation test", did_test_operation_suite_init, did_test_operation_suite_cleanup, NULL, NULL, cases },
    {  NULL,                NULL,                          NULL,                             NULL, NULL, NULL  }
};

CU_SuiteInfo* did_operation_test_suite_info(void)
{
    return suite;
}
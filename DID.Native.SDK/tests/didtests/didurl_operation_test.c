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

static DIDURL *id;
static DID *did;

static void test_didurl_get_did(void)
{
    bool isEqual;

    DID *tempdid = DIDURL_GetDid(id);
    CU_ASSERT_PTR_NOT_NULL(tempdid);

    isEqual = DID_Equals(tempdid, did);
    CU_ASSERT_TRUE(isEqual);
}

static void test_didurl_get_fragment(void)
{
    const char *tempfragment;

    tempfragment = DIDURL_GetFragment(id);
    CU_ASSERT_STRING_EQUAL(tempfragment, fragment);
}

static void test_didurl_tostring(void)
{
    char _idstring[ELA_MAX_DIDURL_LEN];
    CU_ASSERT_STRING_EQUAL(DIDURL_ToString(id, _idstring, sizeof(_idstring), false),
            testid_string);
    CU_ASSERT_STRING_EQUAL(DIDURL_ToString(id, _idstring, sizeof(_idstring), true),
            compact_idstring);
}

static void test_didurl_tostring_error(void)
{
    char _idstring[5];
    CU_ASSERT_PTR_NULL(DIDURL_ToString(id, _idstring, sizeof(_idstring), false));
    CU_ASSERT_PTR_NULL(DIDURL_ToString(id, _idstring, sizeof(_idstring), true));
}

static void test_didurl_compare(void)
{
    int rc;

    DIDURL *comid = DIDURL_New("abc", "def");
    rc = DIDURL_Compare(comid, id);
    DIDURL_Destroy(comid);
    CU_ASSERT_TRUE(rc < 0);

    comid = DIDURL_New("zyx", "def");
    rc = DIDURL_Compare(comid, id);
    DIDURL_Destroy(comid);
    CU_ASSERT_TRUE(rc > 0);

    comid = DIDURL_New(method_specific_string, fragment);
    rc = DIDURL_Compare(comid, id);
    DIDURL_Destroy(comid);
    CU_ASSERT_TRUE(rc == 0);
}

static void test_didurl_equals(void)
{
    bool isEqual;

    DIDURL *equalid = DIDURL_New(method_specific_string, fragment);
    isEqual = DIDURL_Equals(equalid, id);
    DIDURL_Destroy(equalid);
    CU_ASSERT_TRUE(isEqual);

    equalid = DIDURL_New("abc", "def");
    isEqual = DIDURL_Equals(equalid, id);
    DIDURL_Destroy(equalid);
    CU_ASSERT_FALSE(isEqual);
}

static int didurl_test_operation_suite_init(void)
{
    did = DID_FromString(testdid_string);
    if (!did)
        return -1;

    id = DIDURL_FromString(testid_string, NULL);
    if (!id) {
        DID_Destroy(did);
        return -1;
    }

    return  0;
}

static int didurl_test_operation_suite_cleanup(void)
{
    DID_Destroy(did);
    DIDURL_Destroy(id);
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didurl_get_did",                    test_didurl_get_did         },
    {   "test_didurl_get_fragment",               test_didurl_get_fragment    },
    {   "test_didurl_tostring",                   test_didurl_tostring        },
    {   "test_didurl_tostring_error",             test_didurl_tostring_error  },
    {   "test_didurl_compare",                    test_didurl_compare         },
    {   "test_didurl_equals",                     test_didurl_equals          },
    {   NULL,                                     NULL                        }
};

static CU_SuiteInfo suite[] = {
    { "didurl operation test", didurl_test_operation_suite_init, didurl_test_operation_suite_cleanup, NULL, NULL, cases },
    {  NULL,                   NULL,                             NULL,                                NULL, NULL, NULL  }
};

CU_SuiteInfo* didurl_operation_test_suite_info(void)
{
    return suite;
}
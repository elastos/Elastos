#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include "ela_did.h"

#define  TEST_LEN    512

const char *test_id_string;
static DID *did;

static void test_did_get_method(void)
{
    CU_ASSERT_STRING_EQUAL(DID_GetMethod(did), "elastos");
}

static void test_did_get_specificid(void)
{
    CU_ASSERT_STRING_EQUAL(DID_GetMethodSpecificString(did), "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
}

static void test_did_get_didString(void)
{
    char id[128];
    CU_ASSERT_STRING_EQUAL(DID_ToString(did, id, sizeof(id)),
            "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
}

static int did_test_getelem_suite_init(void)
{
    test_id_string = "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN";
    did = DID_FromString(test_id_string);
    if(!did)
        return -1;

    return  0;
}

static int did_test_getelem_suite_cleanup(void)
{
    DID_Destroy(did);
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_did_get_method",                 test_did_get_method      },
    {   "test_did_get_specificid",             test_did_get_specificid  },
    {   "test_did_get_didString",              test_did_get_didString   },
    {   NULL,                                  NULL                     }
};

static CU_SuiteInfo suite[] = {
    {   "did get elem test",    did_test_getelem_suite_init,      did_test_getelem_suite_cleanup,        NULL, NULL, cases },
    {    NULL,         NULL,                                NULL,                                     NULL, NULL, NULL  }
};

CU_SuiteInfo* did_getelem_test_suite_info(void)
{
    return suite;
}
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include "ela_did.h"
#include "did.h"
#include "loader.h"
#include "diddocument.h"

#define  TEST_LEN    512

const char *test_id_string;
static DID *did;

static void test_did_fromString(void)
{
    did = DID_FromString(test_id_string);
    CU_ASSERT_PTR_NOT_NULL_FATAL(did);
}

static void test_did_from_json(void)
{
    DIDDocument *doc = DIDDocument_FromJson(global_did_string);
    if(!doc)
        return;

    did = (DID*)calloc(1, sizeof(DID));
    if (!did) {
        DIDDocument_Destroy(doc);
        return;
    }

    strcpy(did->idstring, doc->did.idstring);
    DIDDocument_Destroy(doc);

    CU_ASSERT_PTR_NOT_NULL_FATAL(did);
}

static int did_test_fromstring_suite_init(void)
{
    test_id_string = "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN";
    return  0;
}

static int did_test_fromstring_suite_cleanup(void)
{
    DID_Destroy(did);
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_did_fromString",             test_did_fromString      },
    {   "test_did_from_json",              test_did_from_json       },
    {   NULL,                              NULL                     }
};

static CU_SuiteInfo suite[] = {
    {   "did test",    did_test_fromstring_suite_init,      did_test_fromstring_suite_cleanup,        NULL, NULL, cases },
    {    NULL,         NULL,                                NULL,                                     NULL, NULL, NULL  }
};

CU_SuiteInfo* did_fromstring_test_suite_info(void)
{
    return suite;
}
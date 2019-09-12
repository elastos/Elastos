#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <crystal.h>
#include <CUnit/Basic.h>
#include <limits.h>

#include "loader.h"
#include "ela_did.h"
#include "did.h"
#include "diddocument.h"
#include "didstore.h"

#define  TEST_LEN    512

static DIDDocument *document;
static DID *did;

static void test_didstore_store_did(void)
{
    int rc;

    rc = DIDStore_StoreDID(document, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(document, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    rc = DIDStore_StoreDID(document, "littlefish");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDStore_DeleteDID(did);
}

static int didstore_store_test_suite_init(void)
{
    char current_path[PATH_MAX];
    document = DIDDocument_FromJson(global_did_string);
    if(!document)
        return -1;

    did = DIDDocument_GetSubject(document);
    if (!did)
        return -1;

    if(!getcwd(current_path, PATH_MAX)) {
        printf("\nCan't get current dir.");
        return -1;
    }

    strcat(current_path, "/servet");
    return DIDStore_Open(current_path);
}

static int didstore_store_test_suite_cleanup(void)
{
    DIDDocument_Destroy(document);
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_store_did",             test_didstore_store_did     },
    {   NULL,                                  NULL                        }
};

static CU_SuiteInfo suite[] = {
    {   "didstore did test",    didstore_store_test_suite_init,    didstore_store_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                  NULL,                         NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_store_did_test_suite_info(void)
{
    return suite;
}
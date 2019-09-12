#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <CUnit/Basic.h>
#include <crystal.h>
#include <limits.h>

#include "loader.h"
#include "ela_did.h"
#include "did.h"
#include "didstore.h"

#define  TEST_LEN    512

static DID *did;

static void test_didstore_open(void)
{
    char current_path[PATH_MAX];

    if(!getcwd(current_path, PATH_MAX)) {
        printf("\nCan't get current dir.");
        return;
    }

    strcat(current_path, "/servet");

    int rc = DIDStore_Open(current_path);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static int didstore_open_test_suite_init(void)
{
    return  0;
}

static int didstore_open_test_suite_cleanup(void)
{
    //DID_Destroy(did);
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_open",       test_didstore_open     },
    {   NULL,                       NULL                   }
};

static CU_SuiteInfo suite[] = {
    {   "didstore open test",    didstore_open_test_suite_init,    didstore_open_test_suite_cleanup,   NULL, NULL, cases },
    {    NULL,                   NULL,                             NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_open_test_suite_info(void)
{
    return suite;
}
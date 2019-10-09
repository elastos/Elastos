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
#include "didstore.h"
#include "diddocument.h"

#define  TEST_LEN    512

static void test_didstore_new_did(void)
{
    DID *did;
    int rc;

    const char *mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";
    rc = DIDStore_InitPrivateIdentity(mnemonic, "", 0);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    DIDDocument *document = DIDStore_NewDID("", "littlefish");
    CU_ASSERT_PTR_NOT_NULL(document);

    did = DIDDocument_GetSubject(document);
    DIDStore_DeleteDID(did);
    DIDDocument_Destroy(document);
}

static int didstore_new_test_suite_init(void)
{
    char current_path[PATH_MAX];
    int rc;

    if(!getcwd(current_path, PATH_MAX)) {
        printf("\nCan't get current dir.");
        return -1;
    }

    strcat(current_path, "/newdid");
    rc = DIDStore_Open(current_path);
    if (rc < 0)
        return -1;

    return 0;
}

static int didstore_new_test_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_new_did",        test_didstore_new_did     },
    {   NULL,                                  NULL                        }
};

static CU_SuiteInfo suite[] = {
    {   "didstore new did test",    didstore_new_test_suite_init,    didstore_new_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                  NULL,                         NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_new_did_test_suite_info(void)
{
    return suite;
}
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
#include "credential.h"
#include "diddocument.h"

#define  TEST_LEN    512

static DID *did;
static Credential *credential;

static void test_didstore_store_cred(void)
{
    int rc;

    rc = DIDStore_StoreCredential(credential, "me");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    DIDStore_DeleteCredential(did, &(credential->id));
}

static int didstore_storecred_test_suite_init(void)
{
    char current_path[PATH_MAX];
    if(!getcwd(current_path, PATH_MAX)) {
        printf("\nCan't get current dir.");
        return -1;
    }

    strcat(current_path, "/servet");
    if(DIDStore_Open(current_path) == -1)
        return -1;

    DIDDocument *doc = DIDDocument_FromJson(global_did_string);
    if(!doc)
        return -1;

    did = (DID*)calloc(1, sizeof(DID));
    if (!did) {
        DIDDocument_Destroy(doc);
        return -1;
    }

    strcpy(did->idstring, doc->did.idstring);

    DIDDocument_Destroy(doc);

    credential = Credential_FromJson(global_cred_string, did);
    if(!credential) {
        DID_Destroy(did);
        return -1;
    }

    return 0;
}

static int didstore_storecred_test_suite_cleanup(void)
{
    DID_Destroy(did);
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_store_cred",            test_didstore_store_cred     },
    {   NULL,                                  NULL                        }
};

static CU_SuiteInfo suite[] = {
    {   "didstore did test",    didstore_storecred_test_suite_init,    didstore_storecred_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                  NULL,                                  NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_store_cred_test_suite_info(void)
{
    return suite;
}
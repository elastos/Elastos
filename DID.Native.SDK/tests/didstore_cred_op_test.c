#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <crystal.h>
#include <CUnit/Basic.h>
#include <limits.h>

//#include "common.h"
#include "loader.h"
#include "ela_did.h"
#include "did.h"
#include "didstore.h"
#include "credential.h"
#include "diddocument.h"

#define  TEST_LEN    512

static DID *did;
static Credential *credential;

int get_cred_hint(CredentialEntry *entry, void *context)
{
    if(!entry)
        return -1;

    printf("\n credential: %s#%s, hint: %s\n", entry->id.did.idstring, entry->id.fragment, entry->hint);
    return 0;
}

static void test_didstore_contain_creds(void)
{
    bool rc;

    rc = DIDStore_ContainsCredentials(did);
    CU_ASSERT_NOT_EQUAL(rc, false);
    rc = DIDStore_ContainsCredential(did, &(credential->id));
    CU_ASSERT_NOT_EQUAL(rc, false);
}

static void test_didstore_list_cred(void)
{
    int rc = DIDStore_ListCredentials(did, get_cred_hint, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didstore_select_cred(void)
{
    int rc = DIDStore_SelectCredentials(did, &(credential->id), "BasicProfileCredential", get_cred_hint, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didstore_load_cred(void)
{
    Credential *cred = DIDStore_LoadCredential(did, &(credential->id));
    CU_ASSERT_PTR_NOT_NULL(cred);

    free(cred);
}

static void test_didstore_delete_cred(void)
{
    if(DIDStore_ContainsCredential(did, &(credential->id)) == true) {
        DIDStore_DeleteCredential(did, &(credential->id));
    }

    bool rc = DIDStore_ContainsCredential(did, &(credential->id));
    CU_ASSERT_NOT_EQUAL(rc, true);
}

static int didstore_cred_op_test_suite_init(void)
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

    return DIDStore_StoreCredential(credential, "me");
}

static int didstore_cred_op_test_suite_cleanup(void)
{
    DIDStore_DeleteDID(did);
    DID_Destroy(did);
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_contain_creds",      test_didstore_contain_creds     },
    {   "test_didstore_list_cred",          test_didstore_list_cred         },
    {   "test_didstore_select_cred",        test_didstore_select_cred       },
    {   "test_didstore_load_cred",          test_didstore_load_cred         },
    {   "test_didstore_delete_cred",        test_didstore_delete_cred       },
    {   NULL,                               NULL                            }
};

static CU_SuiteInfo suite[] = {
    {   "didstore cred op test",    didstore_cred_op_test_suite_init,    didstore_cred_op_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                     NULL,                         NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_cred_op_test_suite_info(void)
{
    return suite;
}
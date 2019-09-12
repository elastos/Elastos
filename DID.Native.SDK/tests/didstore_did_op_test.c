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

static DIDDocument *document;
static DID *did;

int get_did_hint(DIDEntry *entry, void *context)
{
    if(!entry)
        return -1;

    printf("\n did: %s, hint: %s\n", entry->did.idstring, entry->hint);
    free(entry);
    return 0;
}

static void test_didstore_contain_did(void)
{
    bool rc;

    rc = DIDStore_ContainsDID(did);
    CU_ASSERT_NOT_EQUAL(rc, false);
}

static void test_didstore_list_did(void)
{
    int rc = DIDStore_ListDID(get_did_hint, NULL);
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didstore_load_did(void)
{
    DIDDocument *doc = DIDStore_LoadDID(did);
    CU_ASSERT_PTR_NOT_NULL(doc);

    DIDDocument_Destroy(doc);
}

static void test_didstore_delete_did(void)
{
    if(DIDStore_ContainsDID(did) == true) {
        DIDStore_DeleteDID(did);
    }

    bool rc = DIDStore_ContainsDID(did);
    CU_ASSERT_NOT_EQUAL(rc, true);
}

static int didstore_did_op_test_suite_init(void)
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
    if(DIDStore_Open(current_path) == -1)
        return -1;

    return DIDStore_StoreDID(document, "littlefish");
}

static int didstore_did_op_test_suite_cleanup(void)
{
    DIDStore_DeleteDID(did);
    DIDDocument_Destroy(document);
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didstore_contain_did",       test_didstore_contain_did     },
    {   "test_didstore_list_did",          test_didstore_list_did        },
    {   "test_didstore_load_did",          test_didstore_load_did        },
    {   "test_didstore_delete_did",        test_didstore_delete_did      },
    {   NULL,                              NULL                          }
};

static CU_SuiteInfo suite[] = {
    {   "didstore did op test",    didstore_did_op_test_suite_init,    didstore_did_op_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                  NULL,                         NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_did_op_test_suite_info(void)
{
    return suite;
}
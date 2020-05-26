#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <CUnit/Basic.h>
#include <crystal.h>
#include <limits.h>

#include "constant.h"
#include "loader.h"
#include "ela_did.h"
#include "diddocument.h"
#include "didtest_adapter.h"
#include "didstore.h"

static const char *alias = "littlefish";

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return walletpass;
}

static int get_did(DID *did, void *context)
{
    int *count = (int*)context;

    if (!did)
        return 0;

    (*count)++;
    return 0;
}

static void test_didstore_change_password(void)
{
    char alias[ELA_MAX_ALIAS_LEN], _path[PATH_MAX];
    const char *gAlias, *storePath;
    DIDStore *store;
    int rc, count = 0;
    DIDDocument *newdoc;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    rc = TestData_InitIdentity(store);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    for (int i = 0; i < 10; i++) {
        int size = snprintf(alias, sizeof(alias), "my did %d", i);
        if (size < 0 || size > sizeof(alias))
            continue;

        DIDDocument *doc = DIDStore_NewDID(store, storepass, alias);
        if (!doc)
            continue;
        CU_ASSERT_TRUE(DIDDocument_IsValid(doc));

        DID *did = DIDDocument_GetSubject(doc);
        CU_ASSERT_PTR_NOT_NULL(did);

        char *path = get_file_path(_path, PATH_MAX, 7, store->root, PATH_STEP,
                DID_DIR, PATH_STEP, did->idstring, PATH_STEP, DOCUMENT_FILE);
        CU_ASSERT_TRUE(file_exist(path));

        path = get_file_path(_path, PATH_MAX, 7, store->root, PATH_STEP, DID_DIR,
                PATH_STEP, did->idstring, PATH_STEP, META_FILE);
        CU_ASSERT_TRUE(file_exist(path));

        DIDDocument *loaddoc = DIDStore_LoadDID(store, did);
        CU_ASSERT_PTR_NOT_NULL(loaddoc);
        CU_ASSERT_TRUE(DIDDocument_IsValid(loaddoc));

        gAlias = DIDDocument_GetAlias(loaddoc);
        CU_ASSERT_NOT_EQUAL(rc, -1);

        CU_ASSERT_STRING_EQUAL(alias, gAlias);
        CU_ASSERT_STRING_EQUAL(DIDDocument_GetProofSignature(doc),
                DIDDocument_GetProofSignature(loaddoc));

        bool isEquals = DID_Equals(did, DIDDocument_GetSubject(loaddoc));
        CU_ASSERT_TRUE(isEquals);

        DIDDocument_Destroy(doc);
        DIDDocument_Destroy(loaddoc);
    }

    rc = DIDStore_ListDIDs(store, 0, get_did, (void*)&count);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(count, 10);

    count = 0;
    rc = DIDStore_ListDIDs(store, 1, get_did, (void*)&count);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(count, 10);

    count = 0;
    rc = DIDStore_ListDIDs(store, 2, get_did, (void*)&count);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(count, 0);

    //change password
    rc = DIDStore_ChangePassword(store, "newpasswd", storepass);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    count = 0;
    rc = DIDStore_ListDIDs(store, 0, get_did, (void*)&count);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(count, 10);

    count = 0;
    rc = DIDStore_ListDIDs(store, 1, get_did, (void*)&count);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(count, 10);

    count = 0;
    rc = DIDStore_ListDIDs(store, 2, get_did, (void*)&count);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(count, 0);

    newdoc = DIDStore_NewDID(store, "newpasswd", "new");
    CU_ASSERT_PTR_NOT_NULL(newdoc);

    TestData_Free();
}

static void test_didstore_change_with_wrongpassword(void)
{
    char alias[ELA_MAX_ALIAS_LEN], _path[PATH_MAX];
    const char *gAlias, *storePath;
    DIDStore *store;
    int rc, count = 0;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    rc = TestData_InitIdentity(store);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    for (int i = 0; i < 10; i++) {
        int size = snprintf(alias, sizeof(alias), "my did %d", i);
        if (size < 0 || size > sizeof(alias))
            continue;

        DIDDocument *doc = DIDStore_NewDID(store, storepass, alias);
        if (!doc)
            continue;
        CU_ASSERT_TRUE(DIDDocument_IsValid(doc));

        DID *did = DIDDocument_GetSubject(doc);
        CU_ASSERT_PTR_NOT_NULL(did);

        char *path = get_file_path(_path, PATH_MAX, 7, store->root, PATH_STEP,
                DID_DIR, PATH_STEP, did->idstring, PATH_STEP, DOCUMENT_FILE);
        CU_ASSERT_TRUE(file_exist(path));

        path = get_file_path(_path, PATH_MAX, 7, store->root, PATH_STEP, DID_DIR,
                PATH_STEP, did->idstring, PATH_STEP, META_FILE);
        CU_ASSERT_TRUE(file_exist(path));

        DIDDocument *loaddoc = DIDStore_LoadDID(store, did);
        CU_ASSERT_PTR_NOT_NULL(loaddoc);
        CU_ASSERT_TRUE(DIDDocument_IsValid(loaddoc));

        gAlias = DIDDocument_GetAlias(loaddoc);
        CU_ASSERT_NOT_EQUAL(rc, -1);

        CU_ASSERT_STRING_EQUAL(alias, gAlias);
        CU_ASSERT_STRING_EQUAL(DIDDocument_GetProofSignature(doc),
                DIDDocument_GetProofSignature(loaddoc));

        bool isEquals = DID_Equals(did, DIDDocument_GetSubject(loaddoc));
        CU_ASSERT_TRUE(isEquals);

        DIDDocument_Destroy(doc);
        DIDDocument_Destroy(loaddoc);
    }

    rc = DIDStore_ListDIDs(store, 0, get_did, (void*)&count);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(count, 10);

    count = 0;
    rc = DIDStore_ListDIDs(store, 1, get_did, (void*)&count);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(count, 10);

    count = 0;
    rc = DIDStore_ListDIDs(store, 2, get_did, (void*)&count);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(count, 0);

    //change password
    rc = DIDStore_ChangePassword(store, "newpasswd", "wrongpasswd");
    CU_ASSERT_EQUAL(rc, -1);

    TestData_Free();
}


static int didstore_change_password_test_suite_init(void)
{
    return 0;
}

static int didstore_change_password_test_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {  "test_didstore_change_password",            test_didstore_change_password               },
    {  "test_didstore_change_with_wrongpassword",  test_didstore_change_with_wrongpassword     },
    {  NULL,                                       NULL                                        }
};

static CU_SuiteInfo suite[] = {
    {  "didstore change password test",  didstore_change_password_test_suite_init,  didstore_change_password_test_suite_cleanup,   NULL, NULL, cases },
    {  NULL,                     NULL,                              NULL,                                  NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_change_password_test_suite_info(void)
{
    return suite;
}

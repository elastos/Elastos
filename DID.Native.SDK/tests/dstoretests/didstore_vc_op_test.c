#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <crystal.h>
#include <CUnit/Basic.h>
#include <limits.h>

#include "constant.h"
#include "loader.h"
#include "ela_did.h"
#include "did.h"
#include "didmeta.h"
#include "didstore.h"

static int get_vc(DIDURL *id, void *context)
{
    int *count = (int*)context;

    const char *alias;
    int rc;

    if (!id)
        return 0;

    (*count)++;

    alias = DIDURL_GetAlias(id);
    CU_ASSERT_PTR_NOT_NULL(alias);

    if (strcmp(id->fragment, "profile") == 0 ||
            strcmp(id->fragment, "email") == 0 ||
            strcmp(id->fragment, "twitter") == 0 ||
            strcmp(id->fragment, "passport") == 0) {
        CU_ASSERT_TRUE(true);
    } else {
        CU_ASSERT_TRUE(false);
    }

    if (strcmp(alias, "MyProfile") == 0 || strcmp(alias, "Email") == 0 ||
            strcmp(alias, "Twitter") == 0 || strcmp(alias, "Passport") == 0) {
        CU_ASSERT_TRUE(true);
    } else {
        CU_ASSERT_TRUE(false);
    }

    return 0;
}

static void test_didstore_load_vcs(void)
{
    char _path[PATH_MAX];
    const char *storePath, *alias;
    DIDDocument *issuerdoc, *doc;
    DIDStore *store;
    Credential *vc;
    DIDURL *id;
    DID *did;
    int rc;
    bool isEquals;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    rc = TestData_InitIdentity(store);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    issuerdoc = TestData_LoadIssuerDoc();
    doc = TestData_LoadDoc();
    did = DIDDocument_GetSubject(doc);

    vc = TestData_LoadProfileVc();
    rc = Credential_SetAlias(vc, "MyProfile");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    vc = TestData_LoadEmailVc();
    rc = Credential_SetAlias(vc, "Email");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    vc = TestData_LoadTwitterVc();
    rc = Credential_SetAlias(vc, "Twitter");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    vc = TestData_LoadPassportVc();
    rc = Credential_SetAlias(vc, "Passport");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    id = DIDURL_NewByDid(did, "profile");
    CU_ASSERT_PTR_NOT_NULL(id);

    vc = DIDStore_LoadCredential(store, did, id);
    CU_ASSERT_PTR_NOT_NULL(vc);
    alias = Credential_GetAlias(vc);
    CU_ASSERT_PTR_NOT_NULL(alias);
    CU_ASSERT_STRING_EQUAL("MyProfile", alias);
    isEquals = DID_Equals(did, Credential_GetOwner(vc));
    CU_ASSERT_TRUE(isEquals);
    isEquals = DIDURL_Equals(id, Credential_GetId(vc));
    CU_ASSERT_TRUE(isEquals);
    CU_ASSERT_TRUE(Credential_IsValid(vc));
    CU_ASSERT_TRUE(DIDStore_ContainsCredential(store, did, id));
    Credential_Destroy(vc);
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(did, "twitter");
    CU_ASSERT_PTR_NOT_NULL(id);

    vc = DIDStore_LoadCredential(store, did, id);
    CU_ASSERT_PTR_NOT_NULL(vc);
    alias = Credential_GetAlias(vc);
    CU_ASSERT_PTR_NOT_NULL(alias);
    CU_ASSERT_STRING_EQUAL("Twitter", alias);
    isEquals = DID_Equals(did, Credential_GetOwner(vc));
    CU_ASSERT_TRUE(isEquals);
    isEquals = DIDURL_Equals(id, Credential_GetId(vc));
    CU_ASSERT_TRUE(isEquals);
    CU_ASSERT_TRUE(Credential_IsValid(vc));
    CU_ASSERT_TRUE(DIDStore_ContainsCredential(store, did, id));
    Credential_Destroy(vc);
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(did, "notExist");
    CU_ASSERT_PTR_NOT_NULL(id);

    vc = DIDStore_LoadCredential(store, did, id);
    CU_ASSERT_PTR_NULL(vc);
    CU_ASSERT_FALSE(DIDStore_ContainsCredential(store, did, id));
    DIDURL_Destroy(id);

    TestData_Free();
}

static void test_didstore_list_vcs(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    DIDDocument *issuerdoc, *doc;
    DIDStore *store;
    Credential *vc;
    DID *did;
    int rc, count = 0;
    bool isEquals;
    DIDURL vcs[4];

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    rc = TestData_InitIdentity(store);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    issuerdoc = TestData_LoadIssuerDoc();
    doc = TestData_LoadDoc();
    did = DIDDocument_GetSubject(doc);

    vc = TestData_LoadProfileVc();
    rc = Credential_SetAlias(vc, "MyProfile");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    vc = TestData_LoadEmailVc();
    rc = Credential_SetAlias(vc, "Email");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    vc = TestData_LoadTwitterVc();
    rc = Credential_SetAlias(vc, "Twitter");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    vc = TestData_LoadPassportVc();
    rc = Credential_SetAlias(vc, "Passport");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    rc = DIDStore_ListCredentials(store, did, get_vc, (void*)&count);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(count, 4);

    TestData_Free();
}

static void test_didstore_delete_vc(void)
{
    char _path[PATH_MAX];
    const char *storePath, *path;
    DIDDocument *issuerdoc, *doc;
    DIDStore *store;
    Credential *vc;
    DIDURL *id;
    DID *did;
    int rc, count = 0;
    bool isEquals;
    DIDURL vcs[4];
    bool isDeleted;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    rc = TestData_InitIdentity(store);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    issuerdoc = TestData_LoadIssuerDoc();
    doc = TestData_LoadDoc();
    did = DIDDocument_GetSubject(doc);

    vc = TestData_LoadProfileVc();
    rc = Credential_SetAlias(vc, "MyProfile");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    vc = TestData_LoadEmailVc();
    rc = Credential_SetAlias(vc, "Email");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    vc = TestData_LoadTwitterVc();
    rc = Credential_SetAlias(vc, "Twitter");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    vc = TestData_LoadPassportVc();
    rc = Credential_SetAlias(vc, "Passport");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    path = get_file_path(_path, PATH_MAX, 11, store->root, PATH_STEP, DID_DIR,
            PATH_STEP, did->idstring, PATH_STEP, CREDENTIALS_DIR, PATH_STEP,
            "twitter", PATH_STEP, CREDENTIAL_FILE);
    CU_ASSERT_TRUE(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 11, store->root, PATH_STEP, DID_DIR,
            PATH_STEP, did->idstring, PATH_STEP, CREDENTIALS_DIR, PATH_STEP,
            "twitter", PATH_STEP, META_FILE);
    CU_ASSERT_TRUE(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 11, store->root, PATH_STEP, DID_DIR,
            PATH_STEP, did->idstring, PATH_STEP, CREDENTIALS_DIR, PATH_STEP,
            "passport", PATH_STEP, CREDENTIAL_FILE);
    CU_ASSERT_TRUE(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 11, store->root, PATH_STEP, DID_DIR,
            PATH_STEP, did->idstring, PATH_STEP, CREDENTIALS_DIR, PATH_STEP,
            "passport", PATH_STEP, META_FILE);
    CU_ASSERT_TRUE(file_exist(path));

    id = DIDURL_NewByDid(did, "twitter");
    CU_ASSERT_PTR_NOT_NULL(id);
    isDeleted = DIDStore_DeleteCredential(store, did, id);
    CU_ASSERT_TRUE(isDeleted);
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(did, "passport");
    CU_ASSERT_PTR_NOT_NULL(id);
    isDeleted = DIDStore_DeleteCredential(store, did, id);
    CU_ASSERT_TRUE(isDeleted);
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(did, "notExist");
    CU_ASSERT_PTR_NOT_NULL(id);
    isDeleted = DIDStore_DeleteCredential(store, did, id);
    CU_ASSERT_FALSE(isDeleted);
    DIDURL_Destroy(id);

    path = get_file_path(_path, PATH_MAX, 9, store->root, PATH_STEP, DID_DIR,
            PATH_STEP, did->idstring, PATH_STEP, CREDENTIALS_DIR, PATH_STEP,
            "twitter");
    CU_ASSERT_FALSE(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 9, store->root, PATH_STEP, DID_DIR,
            PATH_STEP, did->idstring, PATH_STEP, CREDENTIALS_DIR, PATH_STEP,
            "passport");
    CU_ASSERT_FALSE(file_exist(path));

    id = DIDURL_NewByDid(did, "email");
    CU_ASSERT_PTR_NOT_NULL(id);
    CU_ASSERT_TRUE(DIDStore_ContainsCredential(store, did, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(did, "profile");
    CU_ASSERT_PTR_NOT_NULL(id);
    CU_ASSERT_TRUE(DIDStore_ContainsCredential(store, did, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(did, "twitter");
    CU_ASSERT_PTR_NOT_NULL(id);
    CU_ASSERT_FALSE(DIDStore_ContainsCredential(store, did, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(did, "passport");
    CU_ASSERT_PTR_NOT_NULL(id);
    CU_ASSERT_FALSE(DIDStore_ContainsCredential(store, did, id));
    DIDURL_Destroy(id);

    TestData_Free();
}

static int didstore_vc_op_test_suite_init(void)
{
    return 0;
}

static int didstore_vc_op_test_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {  "test_didstore_load_vcs",       test_didstore_load_vcs     },
    {  "test_didstore_list_vcs",       test_didstore_list_vcs     },
    {  "test_didstore_delete_vc",      test_didstore_delete_vc    },
    {  NULL,                           NULL                       }
};

static CU_SuiteInfo suite[] = {
    { "didstore vc operation test", didstore_vc_op_test_suite_init, didstore_vc_op_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                        NULL,                           NULL,                              NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_vc_op_test_suite_info(void)
{
    return suite;
}
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

static const char *privateindex = "/private/index";
static const char *privatekey = "/private/key";
static const char *storedirroot = "/ids";
static const char *metastring = "/.meta";
static const char *alias = "little fish";

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return walletpass;
}

static void test_didstore_newdid(void)
{
    char _storepath[PATH_MAX], _path[PATH_MAX], newalias[MAX_ALIAS];
    const char *storePath;
    char *path;
    DIDDocument *doc, *loaddoc;
    DIDStore *store;
    bool hasidentity, isEquals;
    int rc;

    storePath = get_store_path(_storepath, "/servet");
    store = TestData_SetupStore(storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    path = get_file_path(_path, PATH_MAX, 2, storePath, storetag);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    const char *newmnemonic = Mnemonic_Generate(0);
    rc = DIDStore_InitPrivateIdentity(store, newmnemonic, "", storepass, 0, false);
    Mnemonic_free((void*)newmnemonic);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_TRUE_FATAL(hasidentity);

    path = get_file_path(_path, PATH_MAX, 2, storePath, privateindex);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 2, storePath, privatekey);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_TRUE_FATAL(DIDDocument_IsValid(doc));

    DID *did = DIDDocument_GetSubject(doc);
    const char *idstring = DID_GetMethodSpecificId(did);

    path = get_file_path(_path, PATH_MAX, 5, storePath, storedirroot, "/",
            (char*)idstring, docstring);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 5, storePath, storedirroot, "/",
            (char*)idstring, metastring);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    rc = DIDDocument_GetAlias(doc, newalias, sizeof(newalias));
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(newalias, alias);

    loaddoc = DIDStore_LoadDID(store, did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(loaddoc);

    isEquals = DID_Equals(DIDDocument_GetSubject(doc), DIDDocument_GetSubject(loaddoc));
    CU_ASSERT_TRUE(isEquals);

    rc = strcmp(doc->proof.signatureValue, loaddoc->proof.signatureValue);
    CU_ASSERT_NOT_EQUAL_FATAL(isEquals, 0);

    CU_ASSERT_TRUE_FATAL(DIDDocument_IsValid(loaddoc));
    TestData_Free();
}

static void test_didstore_newdid_withouAlias(void)
{
    char _storepath[PATH_MAX], _path[PATH_MAX], newalias[MAX_ALIAS];
    const char *storePath;
    char *path;
    DIDDocument *doc, *loaddoc;
    DIDStore *store;
    bool hasidentity, isEquals;
    int rc;

    storePath = get_store_path(_storepath, "/servet");
    store = TestData_SetupStore(storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    path = get_file_path(_path, PATH_MAX, 2, storePath, storetag);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    const char *newmnemonic = Mnemonic_Generate(0);
    rc = DIDStore_InitPrivateIdentity(store, newmnemonic, "", storepass, 0, false);
    Mnemonic_free((void*)newmnemonic);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_TRUE_FATAL(hasidentity);

    path = get_file_path(_path, PATH_MAX, 2, storePath, privateindex);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 2, storePath, privatekey);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    doc = DIDStore_NewDID(store, storepass, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_TRUE_FATAL(DIDDocument_IsValid(doc));

    DID *did = DIDDocument_GetSubject(doc);
    const char *idstring = DID_GetMethodSpecificId(did);

    path = get_file_path(_path, PATH_MAX, 5, storePath, storedirroot, "/",
            (char*)idstring, docstring);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 5, storePath, storedirroot, "/",
            (char*)idstring, metastring);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    rc = DIDDocument_GetAlias(doc, newalias, sizeof(newalias));
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(newalias, "");
}

static void test_didstore_initial_error(void)
{
    char _path[PATH_MAX];
    const char *storePath, *walletDir;
    DIDStore *store;
    DIDAdapter *adapter;

    storePath = get_store_path(_path, "/servet");
    store = DIDStore_Initialize(storePath, NULL);
    CU_ASSERT_PTR_NULL(store);

    walletDir = get_wallet_path(_path, walletdir);
    adapter = TestDIDAdapter_Create(walletDir, walletId, network, resolver, getpassword);
    CU_ASSERT_PTR_NOT_NULL_FATAL(adapter);

    store = DIDStore_Initialize("", adapter);
    CU_ASSERT_PTR_NULL(store);
    DIDStore_Deinitialize();
}

static void test_didstore_privateIdentity_error(void)
{
    char _path[PATH_MAX];
    char _temp[PATH_MAX];
    const char *storePath;
    char *path;
    DIDStore *store;
    bool hasidentity;
    int rc;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    store = DIDStore_GetInstance();
    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    rc = DIDStore_InitPrivateIdentity(store, "", "", storepass, 0, false);
    CU_ASSERT_EQUAL(rc, -1);

    rc = DIDStore_InitPrivateIdentity(store, mnemonic, "", "", 0, false);
    CU_ASSERT_EQUAL(rc, -1);

    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    path = get_file_path(_temp, PATH_MAX, 2, storePath, privateindex);
    CU_ASSERT_FALSE(file_exist(path));

    path = get_file_path(_temp, PATH_MAX, 2, storePath, privatekey);
    CU_ASSERT_FALSE(file_exist(path));

    TestData_Free();
}

static void test_didstore_newdid_emptystore(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    DIDStore *store;
    DIDDocument *doc;
    bool hasidentity;
    int rc;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    doc = DIDStore_NewDID(store, storepass, "little fish");
    CU_ASSERT_PTR_NULL_FATAL(doc);
    DIDDocument_Destroy(doc);

    TestData_Free();
}

static int didstore_initial_test_suite_init(void)
{
    return 0;
}

static int didstore_initial_test_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {  "test_didstore_newdid",                test_didstore_newdid               },
    {  "test_didstore_newdid_withouAlias",    test_didstore_newdid_withouAlias   },
    {  "test_didstore_initial_error",         test_didstore_initial_error        },
    {  "test_didstore_privateIdentity_error", test_didstore_privateIdentity_error},
    {  "test_didstore_newdid_emptystore",     test_didstore_newdid_emptystore    },
    {  NULL,                                  NULL                               }
};

static CU_SuiteInfo suite[] = {
    {  "didstore initial test",  didstore_initial_test_suite_init,  didstore_initial_test_suite_cleanup,   NULL, NULL, cases },
    {  NULL,                     NULL,                              NULL,                                  NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_initial_test_suite_info(void)
{
    return suite;
}
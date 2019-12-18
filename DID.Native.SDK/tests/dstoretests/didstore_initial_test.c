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
#include "didtest_adapter.h"

static const char *privateindex = "/private/index";
static const char *privatekey = "/private/key";
static const char *storedirroot = "/ids";

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return storepass;
}

static void test_didstore_newdid(void)
{
    char _storepath[PATH_MAX], _path[PATH_MAX];
    const char *storePath;
    char *path;
    DIDStore *store;
    DIDDocument *doc, *resolvedoc;
    bool hasidentity;
    int rc;

    storePath = get_store_path(_storepath, "/servet");
    rc = TestData_SetupStore(storePath);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    path = get_file_path(_path, PATH_MAX, 2, storePath, strlen(storePath), storetag,
            strlen(storetag));
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    store = DIDStore_GetInstance();
    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    const char *newmnemonic = Mnemonic_Generate(0);
    rc = DIDStore_InitPrivateIdentity(store, newmnemonic, "", storepass, 0, false);
    Mnemonic_free((void*)newmnemonic);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_TRUE_FATAL(hasidentity);

    path = get_file_path(_path, PATH_MAX, 2, storePath, strlen(storePath), privateindex,
            strlen(privateindex));
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 2, storePath, strlen(storePath), privatekey,
            strlen(privatekey));
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    doc = DIDStore_NewDID(store, storepass, "little fish");
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_TRUE_FATAL(DIDDocument_IsValid(doc));

    DID *did = DIDDocument_GetSubject(doc);
    const char *idstring = DID_GetMethodSpecificId(did);

    path = get_file_path(_path, PATH_MAX, 5, storePath, strlen(storePath),
            storedirroot, strlen(storedirroot), "/", 1, (char*)idstring, strlen(idstring),
            docstring, strlen(docstring));
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    //Todo: check meta file.

    DIDStore_Deinitialize();
}

static void test_didstore_newdid_withouhint(void)
{
    //Todo:
    return;
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

    walletDir = get_wallet_path(_path, "/.wallet");
    adapter = TestDIDAdapter_Create(walletDir, walletId, network, resolver, getpassword);
    CU_ASSERT_PTR_NOT_NULL_FATAL(adapter);

    store = DIDStore_Initialize("", adapter);
    CU_ASSERT_PTR_NULL(store);
    DIDStore_Deinitialize();
}

static void test_didstore_privateIdentity_error(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    char *path;
    DIDStore *store;
    bool hasidentity;
    int rc;

    storePath = get_store_path(_path, "/servet");
    rc = TestData_SetupStore(storePath);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    store = DIDStore_GetInstance();
    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    rc = DIDStore_InitPrivateIdentity(store, "", "", storepass, 0, false);
    CU_ASSERT_EQUAL(rc, -1);

    rc = DIDStore_InitPrivateIdentity(store, mnemonic, "", "", 0, false);
    CU_ASSERT_EQUAL(rc, -1);

    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    path = get_file_path(_path, PATH_MAX, 2, storePath, strlen(storePath), privateindex,
            strlen(privateindex));
    CU_ASSERT_FALSE(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 2, storePath, strlen(storePath), privatekey,
            strlen(privatekey));
    CU_ASSERT_FALSE(file_exist(path));

    DIDStore_Deinitialize();
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
    rc = TestData_SetupStore(storePath);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    store = DIDStore_GetInstance();
    hasidentity = DIDStore_HasPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    doc = DIDStore_NewDID(store, storepass, "little fish");
    CU_ASSERT_PTR_NULL_FATAL(doc);

    DIDStore_Deinitialize();
}

static int didstore_initial_test_suite_init(void)
{
    TestData_Free();
    return 0;
}

static int didstore_initial_test_suite_cleanup(void)
{
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    {  "test_didstore_newdid",                test_didstore_newdid               },
    {  "test_didstore_newdid_withouhint",     test_didstore_newdid_withouhint    },
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
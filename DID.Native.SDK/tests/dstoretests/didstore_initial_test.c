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

static void test_didstore_newdid(void)
{
    char _path[PATH_MAX];
    const char *storePath, *newalias;
    char *path;
    DIDDocument *doc, *loaddoc;
    DIDStore *store;
    bool hasidentity, isEquals;
    int rc;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    path = get_file_path(_path, PATH_MAX, 3, store->root, PATH_STEP, META_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    hasidentity = DIDStore_ContainsPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    const char *newmnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, newmnemonic, "", language, false);
    Mnemonic_Free((void*)newmnemonic);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    hasidentity = DIDStore_ContainsPrivateIdentity(store);
    CU_ASSERT_TRUE_FATAL(hasidentity);

    path = get_file_path(_path, PATH_MAX, 5, store->root, PATH_STEP, PRIVATE_DIR,
            PATH_STEP, INDEX_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 5, store->root, PATH_STEP, PRIVATE_DIR,
            PATH_STEP, HDKEY_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 5, store->root, PATH_STEP, PRIVATE_DIR,
            PATH_STEP, HDKEY_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_TRUE_FATAL(DIDDocument_IsValid(doc));

    DID *did = DIDDocument_GetSubject(doc);
    const char *idstring = DID_GetMethodSpecificId(did);

    path = get_file_path(_path, PATH_MAX, 7, store->root, PATH_STEP, DID_DIR,
            PATH_STEP, (char*)idstring, PATH_STEP, DOCUMENT_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 7, store->root, PATH_STEP, DID_DIR,
            PATH_STEP, (char*)idstring, PATH_STEP, META_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    newalias = DIDDocument_GetAlias(doc);
    CU_ASSERT_NOT_EQUAL(rc, -1);
    CU_ASSERT_STRING_EQUAL(newalias, alias);

    loaddoc = DIDStore_LoadDID(store, did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(loaddoc);

    isEquals = DID_Equals(DIDDocument_GetSubject(doc), DIDDocument_GetSubject(loaddoc));
    CU_ASSERT_TRUE(isEquals);

    rc = strcmp(doc->proof.signatureValue, loaddoc->proof.signatureValue);
    CU_ASSERT_NOT_EQUAL_FATAL(isEquals, 0);

    CU_ASSERT_TRUE_FATAL(DIDDocument_IsValid(loaddoc));

    DIDDocument_Destroy(doc);
    DIDDocument_Destroy(loaddoc);
    TestData_Free();
}

static void test_didstore_newdid_byindex(void)
{
    char _path[PATH_MAX], newalias[ELA_MAX_ALIAS_LEN];
    const char *storePath;
    char *path;
    DIDDocument *doc, *loaddoc;
    DIDStore *store;
    bool hasidentity, isEquals;
    DID did, *ndid;
    int rc;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    path = get_file_path(_path, PATH_MAX, 3, store->root, PATH_STEP, META_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    const char *mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, "", language, false);
    Mnemonic_Free((void*)mnemonic);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    doc = DIDStore_NewDIDByIndex(store, storepass, 0, "did0 by index");
    CU_ASSERT_PTR_NOT_NULL(doc);
    DID_Copy(&did, DIDDocument_GetSubject(doc));

    ndid = DIDStore_GetDIDByIndex(store, 0);
    CU_ASSERT_PTR_NOT_NULL(ndid);

    isEquals = DID_Equals(&did, ndid);
    CU_ASSERT_TRUE(isEquals);
    DIDDocument_Destroy(doc);

    doc = DIDStore_LoadDID(store, ndid);
    CU_ASSERT_PTR_NOT_NULL(doc);
    DID_Destroy(ndid);
    DIDDocument_Destroy(doc);

    doc = DIDStore_NewDID(store, storepass, "did0");
    CU_ASSERT_PTR_NULL(doc);
    CU_ASSERT_TRUE(DIDStore_DeleteDID(store, &did));
    DIDDocument_Destroy(doc);

    doc = DIDStore_NewDID(store, storepass, "did0");
    CU_ASSERT_PTR_NOT_NULL(doc);

    isEquals = DID_Equals(&did, DIDDocument_GetSubject(doc));
    CU_ASSERT_TRUE(isEquals);
    DIDDocument_Destroy(doc);

    TestData_Free();
}

static void test_didstore_newdid_withouAlias(void)
{
    char _storepath[PATH_MAX], _path[PATH_MAX];
    const char *storePath, *newalias;
    char *path;
    DIDDocument *doc, *loaddoc;
    DIDStore *store;
    bool hasidentity, isEquals;
    int rc;

    storePath = get_store_path(_storepath, "/servet");
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    path = get_file_path(_path, PATH_MAX, 3, storePath, PATH_STEP, META_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    hasidentity = DIDStore_ContainsPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    const char *newmnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, newmnemonic, "", language, false);
    Mnemonic_Free((void*)newmnemonic);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    hasidentity = DIDStore_ContainsPrivateIdentity(store);
    CU_ASSERT_TRUE_FATAL(hasidentity);

    path = get_file_path(_path, PATH_MAX, 5, storePath, PATH_STEP, PRIVATE_DIR,
            PATH_STEP, INDEX_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    path = get_file_path(_path, PATH_MAX, 5, storePath, PATH_STEP, PRIVATE_DIR,
            PATH_STEP, HDKEY_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    doc = DIDStore_NewDID(store, storepass, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_TRUE_FATAL(DIDDocument_IsValid(doc));

    DID *did = DIDDocument_GetSubject(doc);
    const char *idstring = DID_GetMethodSpecificId(did);

    path = get_file_path(_path, PATH_MAX, 7, storePath, PATH_STEP, DID_DIR,
            PATH_STEP, (char*)idstring, PATH_STEP, DOCUMENT_FILE);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    newalias = DIDDocument_GetAlias(doc);
    CU_ASSERT_PTR_NOT_NULL(newalias);
    CU_ASSERT_STRING_EQUAL(newalias, "");
}

static void test_didstore_initial_error(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    DIDStore *store;
    DIDAdapter *adapter;

    storePath = get_store_path(_path, "/servet");
    store = DIDStore_Open(storePath, NULL);
    CU_ASSERT_PTR_NULL(store);

    adapter = TestData_GetAdapter(false);
    CU_ASSERT_PTR_NOT_NULL_FATAL(adapter);

    store = DIDStore_Open("", adapter);
    CU_ASSERT_PTR_NULL(store);

    DIDStore_Close(store);
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
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    hasidentity = DIDStore_ContainsPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    rc = DIDStore_InitPrivateIdentity(store, storepass, "", "", language, false);
    CU_ASSERT_EQUAL(rc, -1);

    rc = DIDStore_InitPrivateIdentity(store, "", mnemonic, "", language, false);
    CU_ASSERT_EQUAL(rc, -1);

    hasidentity = DIDStore_ContainsPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    path = get_file_path(_temp, PATH_MAX, 5, storePath, PATH_STEP, PRIVATE_DIR,
            PATH_STEP, INDEX_FILE);
    CU_ASSERT_FALSE(file_exist(path));

    path = get_file_path(_temp, PATH_MAX, 5, storePath, PATH_STEP, PRIVATE_DIR,
            PATH_STEP, HDKEY_FILE);
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
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    hasidentity = DIDStore_ContainsPrivateIdentity(store);
    CU_ASSERT_FALSE(hasidentity);

    doc = DIDStore_NewDID(store, storepass, "little fish");
    CU_ASSERT_PTR_NULL_FATAL(doc);
    DIDDocument_Destroy(doc);

    TestData_Free();
}

static void test_didstore_privateidentity_compatibility(void)
{
    char _path[PATH_MAX];
    char _temp[PATH_MAX];
    const char *storePath;
    char *path;
    DIDStore *store;
    bool isEquals;
    DIDDocument *doc;
    DID did;
    int rc;

    const char *mnemonic = "pact reject sick voyage foster fence warm luggage cabbage any subject carbon";
    const char *ExtendedkeyBase = "xprv9s21ZrQH143K4biiQbUq8369meTb1R8KnstYFAKtfwk3vF8uvFd1EC2s49bMQsbdbmdJxUWRkuC48CXPutFfynYFVGnoeq8LJZhfd9QjvUt";
    const char *passphrase = "helloworld";

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(false, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, passphrase,
            language, false);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    doc = DIDStore_NewDID(store, storepass, "identity test1");
    CU_ASSERT_PTR_NOT_NULL(doc);

    DID_Copy(&did, &doc->did);
    DIDStore_DeleteDID(store, &did);
    DIDDocument_Destroy(doc);

    rc = DIDStore_InitPrivateIdentityFromRootKey(store, storepass, ExtendedkeyBase,
            true);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    doc = DIDStore_NewDID(store, storepass, "identity test2");
    CU_ASSERT_PTR_NOT_NULL(doc);

    isEquals = DID_Equals(&did, &doc->did);
    CU_ASSERT_TRUE(isEquals);
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
    {  "test_didstore_newdid_byindex",        test_didstore_newdid_byindex       },
    {  "test_didstore_newdid_withouAlias",    test_didstore_newdid_withouAlias   },
    {  "test_didstore_initial_error",         test_didstore_initial_error        },
    {  "test_didstore_privateIdentity_error", test_didstore_privateIdentity_error},
    {  "test_didstore_newdid_emptystore",     test_didstore_newdid_emptystore    },
    {  "test_didstore_privateidentity_compatibility", test_didstore_privateidentity_compatibility},
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

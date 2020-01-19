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
#include "constant.h"
#include "ela_did.h"
#include "did.h"
#include "didstore.h"
#include "didtest_adapter.h"

static const char *password = "passwd";

static DIDStore *store;
static DIDAdapter *adapter;

static int get_issuer_cred(DIDURL *id, void *context)
{
    Credential *cred;
    char alias[ELA_MAX_ALIAS_LEN];
    DID *creddid;
    int rc;

    if (!id) {
        return 0;
    }

    creddid = DIDURL_GetDid(id);
    if (!creddid)
        return -1;

    rc = DIDURL_GetAlias(id, alias, sizeof(alias));
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    if (strcmp(alias, "Profile") == 0) {
        cred = DIDStore_LoadCredential(store, creddid, id);
        CU_ASSERT_PTR_NOT_NULL(cred);
        Credential_Destroy(cred);
        return 0;
    }

    return -1;
}

static int get_test_cred(DIDURL *id, void *context)
{
    Credential *cred;
    char alias[ELA_MAX_ALIAS_LEN];
    DID *creddid;
    int rc;

    if (!id)
        return 0;

    creddid = DIDURL_GetDid(id);
    if (!creddid)
        return -1;

    rc = DIDURL_GetAlias(id, alias, sizeof(alias));
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    if (strcmp(alias, "Profile") == 0 || strcmp(alias, "Email") == 0 ||
            strcmp(alias, "Twitter") == 0 || strcmp(alias, "Passport") == 0) {
        cred = DIDStore_LoadCredential(store, creddid, id);
        CU_ASSERT_PTR_NOT_NULL(cred);
        Credential_Destroy(cred);
        return 0;
    }

    return -1;
}

static int get_did(DID *did, void *context)
{
    char alias[ELA_MAX_ALIAS_LEN];
    DIDDocument *doc = NULL;
    int rc;

    if (!did)
        return 0;

    if (DID_GetAlias(did, alias, sizeof(alias)) == -1)
        return -1;

    if (strcmp(alias, "Issuer") == 0) {
        doc = DIDStore_LoadDID(store, did);
        CU_ASSERT_PTR_NOT_NULL(doc);
        DIDDocument_Destroy(doc);

        rc = DIDStore_ListCredentials(store, did, get_issuer_cred, NULL);
        CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
        return 0;
    }

    if (strcmp(alias, "Test") == 0) {
        doc = DIDStore_LoadDID(store, did);
        CU_ASSERT_PTR_NOT_NULL(doc);
        DIDDocument_Destroy(doc);

        rc = DIDStore_ListCredentials(store, did, get_test_cred, NULL);
        CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
        return 0;
    }

    return -1;
}

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return walletpass;
}

static void test_openstore_file_exit(void)
{
    char _path[PATH_MAX], mnemonic[ELA_MAX_MNEMONIC_LEN];
    char *path;
    bool hasidentity;
    int rc;

    hasidentity = DIDStore_ContainsPrivateIdentity(store);
    CU_ASSERT_TRUE(hasidentity);

    path = get_file_path(_path, PATH_MAX, 2, teststore_path, privateindex);
    CU_ASSERT_TRUE_FATAL(file_exist(path));

    rc = DIDStore_ExportMnemonic(store, password, mnemonic, sizeof(mnemonic));
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    rc = DIDStore_ListDID(store, get_did, NULL);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
}

static void test_openstore_newdid(void)
{
    DIDDocument *doc;
    DID *did;
    int rc;

    doc = DIDStore_NewDID(store, password, "");
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);

    did = DIDDocument_GetSubject(doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(did);

    DIDStore_DeleteDID(store, did);
    DIDDocument_Destroy(doc);
}

static void test_openstore_newdid_with_wrongpw(void)
{
    DIDDocument *doc;
    DID *did;

    doc = DIDStore_NewDID(store, "1234", "");
    CU_ASSERT_PTR_NULL(doc);

    did = DIDDocument_GetSubject(doc);
    DIDStore_DeleteDID(store, did);
    DIDDocument_Destroy(doc);
}

static int didstore_openstore_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *walletDir;

    walletDir = get_wallet_path(_path, walletdir);
    adapter = TestDIDAdapter_Create(walletDir, walletId, network, resolver, getpassword);
    if (!adapter)
        return -1;

    store = DIDStore_Initialize(teststore_path, adapter);
    if (!store)
        return -1;

    return 0;
}

static int didstore_openstore_test_suite_cleanup(void)
{
    TestDIDAdapter_Destroy(adapter);
    DIDStore_Deinitialize();
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_openstore_file_exit",             test_openstore_file_exit           },
    { "test_openstore_newdid_with_wrongpw",   test_openstore_newdid_with_wrongpw },
    { "test_openstore_newdid",                test_openstore_newdid              },
    { NULL,                                   NULL                               }
};

static CU_SuiteInfo suite[] = {
    { "didstore open store test", didstore_openstore_test_suite_init, didstore_openstore_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                      NULL,                               NULL,                               NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_openstore_test_suite_info(void)
{
    return suite;
}
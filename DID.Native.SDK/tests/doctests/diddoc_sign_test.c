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
#include "didtest_adapter.h"
#include "ela_did.h"
#include "HDkey.h"

#define SIGNATURE_BYTES         64

static DIDDocument *document;
static DID *did;
static DIDStore *store;
static const char *data = "abcdefghijklmnopqrstuvwxyz";
static const char *wdata = "abc";
static char signature[SIGNATURE_BYTES * 2 + 16];

static void test_diddoc_sign(void)
{
    int rc;
    DIDURL *signkey;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    rc = DIDDocument_Sign(document, signkey, storepass, signature, 1,
            (unsigned char*)data, strlen(data));
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_diddoc_verify(void)
{
    int rc;
    DIDURL *signkey;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    rc = DIDDocument_Verify(document, signkey, signature, 1,
            (unsigned char*)data, strlen(data));
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_diddoc_verify_by_wrong(void)
{
    int rc;
    DIDURL *signkey;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    rc = DIDDocument_Verify(document, signkey, signature, 1,
             (unsigned char*)wdata, strlen(wdata));
    CU_ASSERT_NOT_EQUAL(rc, 0);
}

static int diddoc_sign_test_suite_init(void)
{
    int rc;
    char _path[PATH_MAX];
    const char *storePath, *mnemonic;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(storePath);
    if (!store)
        return -1;

    mnemonic = Mnemonic_Generate(0);
    printf("\n#### mnemonic: %s\n", mnemonic);
    rc = DIDStore_InitPrivateIdentity(store, mnemonic, "", storepass, 0, true);
    if (rc < 0) {
        TestData_Free();
        return -1;
    }

    document = DIDStore_NewDID(store, storepass, "littlefish");
    if(!document) {
        TestData_Free();
        return -1;
    }

    return 0;
}

static int diddoc_sign_test_suite_cleanup(void)
{
    DIDDocument_Destroy(document);
    TestData_Free();

    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_diddoc_sign",                 test_diddoc_sign               },
    {   "test_diddoc_verify",               test_diddoc_verify             },
    {   "test_diddoc_verify_by_wrong",      test_diddoc_verify_by_wrong    },
    {   NULL,                               NULL                           }
};

static CU_SuiteInfo suite[] = {
    {   "diddoc sign test",    diddoc_sign_test_suite_init,   diddoc_sign_test_suite_cleanup,   NULL, NULL, cases },
    {    NULL,                 NULL,                          NULL,                             NULL, NULL, NULL  }
};

CU_SuiteInfo* diddoc_sign_test_suite_info(void)
{
    return suite;
}
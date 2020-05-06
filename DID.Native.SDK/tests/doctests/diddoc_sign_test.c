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
static DIDURL *keyid;
static DIDStore *store;

static void test_diddoc_sign_verify(void)
{
    uint8_t data[124];
    char signature[SIGNATURE_BYTES * 2 + 16];
    int rc;

    for (int i = 0; i < 10; i++) {
        memset(data, i, sizeof(data));

        rc = DIDDocument_Sign(document, keyid, storepass, signature, 1, data, sizeof(data));
        CU_ASSERT_NOT_EQUAL(rc, -1);

        rc = DIDDocument_Verify(document, keyid, signature, 1, data, sizeof(data));
        CU_ASSERT_NOT_EQUAL(rc, -1);

        data[0] = 0xFF;
        rc = DIDDocument_Verify(document, keyid, signature, 1, data, sizeof(data));
        CU_ASSERT_EQUAL(rc, -1);
    }
}

static void test_diddoc_digest_sign_verify(void)
{
    uint8_t digest[32];
    char signature[SIGNATURE_BYTES * 2 + 16];
    int rc;

    for (int i = 0; i < 10; i++) {
        memset(digest, i, sizeof(digest));

        rc = DIDDocument_SignDigest(document, keyid, storepass, signature, digest, sizeof(digest));
        CU_ASSERT_NOT_EQUAL(rc, -1);

        rc = DIDDocument_VerifyDigest(document, keyid, signature, digest, sizeof(digest));
        CU_ASSERT_NOT_EQUAL(rc, -1);

        digest[0] = 0xFF;
        rc = DIDDocument_VerifyDigest(document, keyid, signature, digest, sizeof(digest));
        CU_ASSERT_EQUAL(rc, -1);
    }
}

static int diddoc_sign_test_suite_init(void)
{
    int rc;
    char _path[PATH_MAX];
    const char *storePath, *mnemonic;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(false, storePath);
    if (!store)
        return -1;

    document = TestData_LoadDoc();
    if (!document) {
        TestData_Free();
        return -1;
    }

    keyid = DIDDocument_GetDefaultPublicKey(document);
    if (!keyid) {
        TestData_Free();
        return -1;
    }

    return 0;
}

static int diddoc_sign_test_suite_cleanup(void)
{
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_diddoc_sign_verify",           test_diddoc_sign_verify           },
    {   "test_diddoc_digest_sign_verify",    test_diddoc_digest_sign_verify    },
    {   NULL,                                NULL                              }
};

static CU_SuiteInfo suite[] = {
    { "diddoc sign test",  diddoc_sign_test_suite_init,  diddoc_sign_test_suite_cleanup, NULL, NULL, cases },
    {    NULL,             NULL,                         NULL,                           NULL, NULL, NULL  }
};

CU_SuiteInfo* diddoc_sign_test_suite_info(void)
{
    return suite;
}
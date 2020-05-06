#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <limits.h>

#include <CUnit/Basic.h>
#include "ela_did.h"
#include "loader.h"
#include "constant.h"
#include "diddocument.h"

static DIDDocument *issuerdoc;
static DID *issuerid;
static DIDURL *signkey;
static DIDStore *store;

static void test_issuer_create(void)
{
    Issuer *issuer;
    bool isequal;

    issuer = Issuer_Create(issuerid, signkey, store);
    CU_ASSERT_PTR_NOT_NULL_FATAL(issuer);

    isequal = DID_Equals(issuerid, Issuer_GetSigner(issuer));
    CU_ASSERT_TRUE(isequal);

    isequal = DIDURL_Equals(signkey, Issuer_GetSignKey(issuer));
    CU_ASSERT_TRUE(isequal);

    Issuer_Destroy(issuer);
}

static void test_issuer_create_without_key(void)
{
    Issuer *issuer;
    bool isequal;

    issuer = Issuer_Create(issuerid, NULL, store);
    CU_ASSERT_PTR_NOT_NULL_FATAL(issuer);

    isequal = DID_Equals(issuerid, Issuer_GetSigner(issuer));
    CU_ASSERT_TRUE(isequal);

    isequal = DIDURL_Equals(signkey, Issuer_GetSignKey(issuer));
    CU_ASSERT_TRUE(isequal);

    Issuer_Destroy(issuer);
}

static void test_issuer_create_with_invalidkey1(void)
{
    char pkbase[MAX_PUBLICKEY_BASE58];
    const char *publickeybase;
    DIDDocumentBuilder *builder;
    DIDURL *keyid;
    Issuer *issuer;
    DIDDocument *doc;
    int rc;

    builder = DIDDocument_Edit(issuerdoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(builder);

    publickeybase = Generater_Publickey(pkbase, sizeof(pkbase));
    CU_ASSERT_PTR_NOT_NULL_FATAL(publickeybase);

    keyid = DIDURL_NewByDid(DIDDocument_GetSubject(issuerdoc), "testkey");
    CU_ASSERT_PTR_NOT_NULL_FATAL(keyid);

    rc = DIDDocumentBuilder_AddAuthenticationKey(builder, keyid, publickeybase);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL(doc);
    CU_ASSERT_TRUE(DIDDocument_IsValid(doc));
    DIDDocument_Destroy(doc);

    issuer = Issuer_Create(issuerid, keyid, store);
    DIDURL_Destroy(keyid);

    CU_ASSERT_PTR_NULL(issuer);
    Issuer_Destroy(issuer);
}

static void test_issuer_create_with_invalidkey2(void)
{
    DIDURL *key;
    Issuer *issuer;

    key = DIDURL_NewByDid(issuerid, "key2");
    CU_ASSERT_PTR_NOT_NULL_FATAL(key);

    issuer = Issuer_Create(issuerid, key, store);
    CU_ASSERT_PTR_NULL(issuer);

    Issuer_Destroy(issuer);
}

static int issuer_create_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    int rc;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(false, storePath);
    if (!store)
        return -1;

    issuerdoc = TestData_LoadIssuerDoc();
    if (!issuerdoc) {
        TestData_Free();
        return -1;
    }

    rc = DIDStore_StoreDID(store, issuerdoc, "issuer doc");
    if (rc < 0) {
        TestData_Free();
        return rc;
    }

    issuerid = DIDDocument_GetSubject(issuerdoc);
    if (!issuerid) {
        TestData_Free();
        return -1;
    }

    signkey = DIDDocument_GetDefaultPublicKey(issuerdoc);
    if (!signkey) {
        TestData_Free();
        return -1;
    }

    return 0;
}

static int issuer_create_test_suite_cleanup(void)
{
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_issuer_create",                     test_issuer_create                     },
    { "test_issuer_create_without_key",         test_issuer_create_without_key         },
    { "test_issuer_create_with_invalidkey1",    test_issuer_create_with_invalidkey1    },
    { "test_issuer_create_with_invalidkey2",    test_issuer_create_with_invalidkey2    },
    { NULL,                                     NULL                                   }
};

static CU_SuiteInfo suite[] = {
    { "issuer create test", issuer_create_test_suite_init, issuer_create_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                NULL,                          NULL,                             NULL, NULL, NULL  }
};


CU_SuiteInfo* issuer_create_test_suite_info(void)
{
    return suite;
}

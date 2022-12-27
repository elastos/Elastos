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
static DIDStore *store;

static DIDDocument *local_doc(DID *did)
{
    return DIDStore_LoadDID(store, did);
}

static void test_vc_local_verify(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    DIDDocument *doc;
    DID owner, kyc;
    time_t expires;
    int rc;

    storePath = get_store_path(_path, "/localverify");
    store = TestData_SetupStore(true, storePath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    const char *newmnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, newmnemonic, "", language, false);
    Mnemonic_Free((void*)newmnemonic);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_TRUE_FATAL(DIDDocument_IsValid(doc));

    DID_Copy(&owner, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    doc = DIDStore_NewDID(store, storepass, alias);
    CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
    CU_ASSERT_TRUE_FATAL(DIDDocument_IsValid(doc));

    DID_Copy(&kyc, DIDDocument_GetSubject(doc));
    expires = DIDDocument_GetExpires(doc);

    Issuer *issuer = Issuer_Create(&kyc, NULL, store);
    CU_ASSERT_PTR_NOT_NULL_FATAL(issuer);

    DIDURL *credid = DIDURL_NewByDid(&owner, "kyccredential");
    CU_ASSERT_PTR_NOT_NULL(credid);

    const char *types[] = {"BasicProfileCredential", "PhoneCredential"};
    Property props[7];
    props[0].key = "name";
    props[0].value = "John";
    props[1].key = "gender";
    props[1].value = "Male";
    props[2].key = "nation";
    props[2].value = "Singapore";
    props[3].key = "language";
    props[3].value = "English";
    props[4].key = "email";
    props[4].value = "john@example.com";
    props[5].key = "twitter";
    props[5].value = "@john";
    props[6].key = "phone";
    props[6].value = "132780456";

    Credential *vc = Issuer_CreateCredential(issuer, &owner, credid, types, 2, props, 7,
            expires, storepass);
    Issuer_Destroy(issuer);
    DIDURL_Destroy(credid);

    CU_ASSERT_FALSE(Credential_IsValid(vc));

    DIDBackend_SetLocalResolveHandle(local_doc);
    CU_ASSERT_TRUE(Credential_IsValid(vc));

    DIDBackend_SetLocalResolveHandle(NULL);
    CU_ASSERT_FALSE(Credential_IsValid(vc));

    DIDDocument_Destroy(doc);
    Credential_Destroy(vc);
    TestData_Free();
}

static int vc_local_verify_test_suite_init(void)
{
    return 0;
}

static int vc_local_verify_test_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {  "test_vc_local_verify",   test_vc_local_verify },
    {  NULL,                     NULL                 }
};

static CU_SuiteInfo suite[] = {
    { "vc local verify test", vc_local_verify_test_suite_init,  vc_local_verify_test_suite_cleanup,   NULL, NULL, cases },
    { NULL,                   NULL,                             NULL,                                 NULL, NULL, NULL  }
};

CU_SuiteInfo* vc_local_verify_test_suite_info(void)
{
    return suite;
}

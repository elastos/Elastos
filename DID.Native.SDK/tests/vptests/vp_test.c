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
#include "did.h"
#include "credential.h"

static const char *PresentationType = "VerifiablePresentation";
static DIDDocument *issuerdoc;
static DIDDocument *testdoc;
static DIDStore *store;

static void test_vp_getelem(void)
{
    Presentation *vp;
    bool isEqual;
    ssize_t size;
    Credential *creds[4], **cred;
    DIDURL*id;
    DID *signer;

    vp = TestData_LoadVp();
    CU_ASSERT_PTR_NOT_NULL_FATAL(vp);

    CU_ASSERT_NOT_EQUAL_FATAL(Presentation_GetType(vp), PresentationType);
    isEqual = DID_Equals(DIDDocument_GetSubject(testdoc), Presentation_GetSigner(vp));
    CU_ASSERT_TRUE(isEqual);

    size = Presentation_GetCredentialCount(vp);
    CU_ASSERT_EQUAL(size, 4);

    size = Presentation_GetCredentials(vp, creds, sizeof(creds));
    CU_ASSERT_EQUAL(size, 4);

    cred = creds;
    for (int i = 0; i < size; i++, cred++) {
        isEqual = DID_Equals(DIDDocument_GetSubject(testdoc), Credential_GetOwner(*cred));
        CU_ASSERT_TRUE(isEqual);

        const char *fragment = DIDURL_GetFragment(Credential_GetId(*cred));
        CU_ASSERT_PTR_NOT_NULL(fragment);

        CU_ASSERT_TRUE(!strcmp(fragment, "profile") || !strcmp(fragment, "email") ||
                 !strcmp(fragment, "twitter") || !strcmp(fragment, "passport"));
    }

    signer = Presentation_GetSigner(vp);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signer);

    id = DIDURL_NewByDid(signer, "profile");
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    CU_ASSERT_PTR_NOT_NULL(Presentation_GetCredential(vp, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(signer, "email");
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    CU_ASSERT_PTR_NOT_NULL(Presentation_GetCredential(vp, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(signer, "twitter");
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    CU_ASSERT_PTR_NOT_NULL(Presentation_GetCredential(vp, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(signer, "passport");
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    CU_ASSERT_PTR_NOT_NULL(Presentation_GetCredential(vp, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(signer, "notexist");
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    CU_ASSERT_PTR_NULL(Presentation_GetCredential(vp, id));
    DIDURL_Destroy(id);

    CU_ASSERT_TRUE(Presentation_IsGenuine(vp));
    CU_ASSERT_TRUE(Presentation_IsValid(vp));
}

static void test_vp_parse(void)
{
    Presentation *vp, *normvp;
    bool isEqual;
    const char *data;

    vp = TestData_LoadVp();
    CU_ASSERT_PTR_NOT_NULL_FATAL(vp);
    CU_ASSERT_TRUE(Presentation_IsGenuine(vp));
    CU_ASSERT_TRUE(Presentation_IsValid(vp));

    normvp = Presentation_FromJson(TestData_LoadVpNormJson());
    CU_ASSERT_PTR_NOT_NULL_FATAL(normvp);
    CU_ASSERT_TRUE(Presentation_IsGenuine(normvp));
    CU_ASSERT_TRUE(Presentation_IsValid(normvp));

    CU_ASSERT_TRUE(!strcmp(TestData_LoadVpNormJson(), Presentation_ToJson(normvp, true)));
    CU_ASSERT_TRUE(!strcmp(TestData_LoadVpNormJson(), Presentation_ToJson(vp, true)));
}

static void test_vp_create(void)
{
    Presentation *vp;
    DID *did;
    Credential *creds[4], **cred;
    bool isEqual;
    ssize_t size;
    DIDURL *id;
    DID *signer;

    did = DIDDocument_GetSubject(testdoc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(did);

    vp = Presentation_Create(did, NULL, store, storepass, "873172f58701a9ee686f0630204fee59",
            "https://example.com/", 4, TestData_LoadProfileVc(), TestData_LoadEmailVc(),
            TestData_LoadPassportVc(), TestData_LoadTwitterVc());
    CU_ASSERT_PTR_NOT_NULL_FATAL(vp);

    CU_ASSERT_NOT_EQUAL_FATAL(Presentation_GetType(vp), PresentationType);
    isEqual = DID_Equals(did, Presentation_GetSigner(vp));
    CU_ASSERT_TRUE(isEqual);

    size = Presentation_GetCredentialCount(vp);
    CU_ASSERT_EQUAL(size, 4);

    size = Presentation_GetCredentials(vp, creds, sizeof(creds));
    CU_ASSERT_EQUAL(size, 4);

    cred = creds;
    for (int i = 0; i < size; i++, cred++) {
        isEqual = DID_Equals(DIDDocument_GetSubject(testdoc), Credential_GetOwner(*cred));
        CU_ASSERT_TRUE(isEqual);

        const char *fragment = DIDURL_GetFragment(Credential_GetId(*cred));
        CU_ASSERT_PTR_NOT_NULL(fragment);

        CU_ASSERT_TRUE(!strcmp(fragment, "profile") || !strcmp(fragment, "email") ||
                 !strcmp(fragment, "twitter") || !strcmp(fragment, "passport"));
    }

    signer = Presentation_GetSigner(vp);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signer);

    id = DIDURL_NewByDid(signer, "profile");
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    CU_ASSERT_PTR_NOT_NULL(Presentation_GetCredential(vp, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(signer, "email");
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    CU_ASSERT_PTR_NOT_NULL(Presentation_GetCredential(vp, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(signer, "twitter");
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    CU_ASSERT_PTR_NOT_NULL(Presentation_GetCredential(vp, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(signer, "passport");
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    CU_ASSERT_PTR_NOT_NULL(Presentation_GetCredential(vp, id));
    DIDURL_Destroy(id);

    id = DIDURL_NewByDid(signer, "notexist");
    CU_ASSERT_PTR_NOT_NULL_FATAL(id);
    CU_ASSERT_PTR_NULL(Presentation_GetCredential(vp, id));
    DIDURL_Destroy(id);

    CU_ASSERT_TRUE(Presentation_IsGenuine(vp));
    CU_ASSERT_TRUE(Presentation_IsValid(vp));
}

static int vp_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    int rc;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(false, storePath);
    if (!store)
        return -1;

    testdoc = TestData_LoadDoc();
    if (!testdoc) {
        TestData_Free();
        return -1;
    }

    issuerdoc = TestData_LoadIssuerDoc();
    if (!issuerdoc) {
        TestData_Free();
        return -1;
    }

    if (DIDStore_StoreDID(store, testdoc, "vp test") == -1) {
        TestData_Free();
        return -1;
    }

    return 0;
}

static int vp_test_suite_cleanup(void)
{
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_vp_getelem",            test_vp_getelem    },
    { "test_vp_parse",              test_vp_parse      },
    { "test_vp_create",             test_vp_create     },
    { NULL,                         NULL               }
};

static CU_SuiteInfo suite[] = {
    { "presentation test",  vp_test_suite_init, vp_test_suite_cleanup,  NULL, NULL, cases },
    {  NULL,                NULL,               NULL,                   NULL, NULL, NULL  }
};


CU_SuiteInfo* vp_test_suite_info(void)
{
    return suite;
}

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
#include "credential.h"

static DIDDocument *issuerdoc;
static DID *issuerid;
static Issuer *issuer;
static DIDStore *store;

static bool has_type(const char **types, size_t size, const char *type)
{
    if (!types || size <= 0 || !type || !*type)
        return false;

    for (int i = 0; i < size; i++) {
        if (!strcmp(types[i], type))
            return true;
    }

    return false;
}

static void test_issuer_issuevc(void)
{
    Credential *vc;
    DIDDocument *doc;
    DIDStore *store;
    DID *did, *vcdid;
    time_t expires;
    bool isEquals;
    ssize_t size;
    int rc;

    doc = TestData_LoadDoc();
    rc = DIDStore_StoreDID(store, doc, "credential doc");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    did = DIDDocument_GetSubject(doc);
    expires = DIDDocument_GetExpires(doc);

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

    vc = Issuer_CreateCredential(issuer, did, "kyccredential", types, 2, props, 7,
            expires, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(vc);
    CU_ASSERT_FALSE(Credential_IsExpired(vc));
    CU_ASSERT_TRUE(Credential_IsGenuine(vc));
    CU_ASSERT_TRUE(Credential_IsValid(vc));

    vcdid = DIDURL_GetDid(Credential_GetId(vc));
    isEquals = DID_Equals(vcdid, did);
    CU_ASSERT_TRUE(isEquals);
    isEquals = DID_Equals(Credential_GetOwner(vc), did);
    CU_ASSERT_TRUE(isEquals);
    isEquals = DID_Equals(Credential_GetIssuer(vc), issuerid);
    CU_ASSERT_TRUE(isEquals);

    CU_ASSERT_EQUAL(Credential_GetTypeCount(vc), 2);
    const char *tmptypes[2];
    size = Credential_GetTypes(vc, tmptypes, 2);
    CU_ASSERT_EQUAL(size, 2);
    CU_ASSERT_TRUE(has_type(tmptypes, 2, "BasicProfileCredential"));
    CU_ASSERT_TRUE(has_type(tmptypes, 2, "PhoneCredential"));

    CU_ASSERT_EQUAL(Credential_GetPropertyCount(vc), 7);
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "name"), "John");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "gender"), "Male");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "nation"), "Singapore");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "language"), "English");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "email"), "john@example.com");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "twitter"), "@john");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "phone"), "132780456");

    Credential_Destroy(vc);
}

static void test_issuer_issueselfvc(void)
{
    Credential *vc;
    DIDDocument *doc;
    DID *did, *vcdid;
    time_t expires;
    bool isEquals;
    ssize_t size;

    expires = DIDDocument_GetExpires(issuerdoc);

    const char *types[] = {"BasicProfileCredential", "PhoneCredential",
            "SelfProclaimedCredential"};
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

    vc = Issuer_CreateCredential(issuer, issuerid, "mycredential", types, 3,
            props, 7, expires, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(vc);
    CU_ASSERT_FALSE(Credential_IsExpired(vc));
    CU_ASSERT_TRUE(Credential_IsGenuine(vc));
    CU_ASSERT_TRUE(Credential_IsValid(vc));

    vcdid = DIDURL_GetDid(Credential_GetId(vc));
    isEquals = DID_Equals(vcdid, issuerid);
    CU_ASSERT_TRUE(isEquals);
    isEquals = DID_Equals(Credential_GetOwner(vc), issuerid);
    CU_ASSERT_TRUE(isEquals);
    isEquals = DID_Equals(Credential_GetIssuer(vc), issuerid);
    CU_ASSERT_TRUE(isEquals);

    CU_ASSERT_EQUAL(Credential_GetTypeCount(vc), 3);
    const char *tmptypes[3];
    size = Credential_GetTypes(vc, tmptypes, 3);
    CU_ASSERT_EQUAL(size, 3);
    CU_ASSERT_TRUE(has_type(tmptypes, 3, "BasicProfileCredential"));
    CU_ASSERT_TRUE(has_type(tmptypes, 3, "PhoneCredential"));
    CU_ASSERT_TRUE(has_type(tmptypes, 3, "SelfProclaimedCredential"));

    CU_ASSERT_EQUAL(Credential_GetPropertyCount(vc), 7);
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "name"), "John");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "gender"), "Male");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "nation"), "Singapore");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "language"), "English");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "email"), "john@example.com");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "twitter"), "@john");
    CU_ASSERT_STRING_EQUAL(Credential_GetProperty(vc, "phone"), "132780456");

    Credential_Destroy(vc);
}

static int issuer_issuevc_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    DIDURL *signkey;
    int rc;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(storePath);
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

    issuer = Issuer_Create(issuerid, signkey, store);
    if (!issuer) {
        TestData_Free();
        return -1;
    }

    return 0;
}

static int issuer_issuevc_test_suite_cleanup(void)
{
    Issuer_Destroy(issuer);
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_issuer_issuevc",                   test_issuer_issuevc       },
    { "test_issuer_issueselfvc",               test_issuer_issueselfvc   },
    { NULL,                                    NULL                      }
};

static CU_SuiteInfo suite[] = {
    { "issuer issue credential test", issuer_issuevc_test_suite_init, issuer_issuevc_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                          NULL,                          NULL,                               NULL, NULL, NULL  }
};


CU_SuiteInfo* issuer_issuevc_test_suite_info(void)
{
    return suite;
}

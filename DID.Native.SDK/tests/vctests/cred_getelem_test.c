#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <CUnit/Basic.h>
#include <limits.h>

#include "loader.h"
#include "ela_did.h"
#include "constant.h"

static Credential *credential;

static void test_cred_get_id(void)
{
    DIDURL *cred_id = Credential_GetId(credential);
    CU_ASSERT_PTR_NOT_NULL_FATAL(cred_id);

    DID *cred_did = DIDURL_GetDid(cred_id);
    CU_ASSERT_STRING_EQUAL_FATAL(DID_GetMethodSpecificId(cred_did), method_specific_string);
}

static void test_cred_get_type_count(void)
{
    ssize_t size = Credential_GetTypeCount(credential);
    CU_ASSERT_EQUAL(size, 3);
}

static void test_cred_get_types(void)
{
    const char *types[3];

    ssize_t size = Credential_GetTypeCount(credential);
    CU_ASSERT_EQUAL(size, 3);

    size = Credential_GetTypes(credential, types, size);
    CU_ASSERT_EQUAL(size, 3);
    return;
}

static void test_cred_get_issuer(void)
{
    DID *issuer = Credential_GetIssuer(credential);
    CU_ASSERT_PTR_NOT_NULL(issuer);
}

static void test_cred_get_issuance_date(void)
{
    time_t time = Credential_GetIssuanceDate(credential);
    CU_ASSERT_NOT_EQUAL(time, 0);
}

static void test_cred_get_expirate_data(void)
{
    DIDStore *store;
    DIDDocument *doc;

    doc = DIDDocument_FromJson(TestData_LoadIssuerJson());
    store = DIDStore_GetInstance();
    int rc = DIDStore_StoreDID(store, doc, "");
    time_t time = Credential_GetExpirationDate(credential);
    CU_ASSERT_NOT_EQUAL(time, 0);
}

static void test_cred_get_property_count(void)
{
    ssize_t size = Credential_GetPropertyCount(credential);
    CU_ASSERT_EQUAL(size, 2);
}

static void test_cred_get_properties(void)
{
    Property *properties[2];

    ssize_t size = Credential_GetPropertyCount(credential);
    CU_ASSERT_EQUAL(size, 2);

    size = Credential_GetProperties(credential, properties, size);
    CU_ASSERT_EQUAL(size, 2);
    return;
}

static void test_cred_get_property(void)
{
    const char *value = Credential_GetProperty(credential, "email");
    CU_ASSERT_PTR_NOT_NULL(value);
}

static void test_cred_add_property(void)
{
    ssize_t size = Credential_AddProperty(credential, "phone", "13188673423");
    CU_ASSERT_EQUAL(size, 3);
}

static void test_cred_get_proof_method(void)
{
    DIDURL *method = Credential_GetProofMethod(credential);
    CU_ASSERT_PTR_NOT_NULL(method);
}

static void test_cred_get_proof_type(void)
{
    const char *type = Credential_GetProofType(credential);
    CU_ASSERT_PTR_NOT_NULL(type);
}

static void test_cred_proof_signature(void)
{
    const char *sign = Credential_GetProofSignture(credential);
    CU_ASSERT_PTR_NOT_NULL(sign);
}

static int cred_getelem_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;
    DIDStore *store;
    DIDDocument *document;
    int rc;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(storePath);
    if (!store)
        return -1;

    document = DIDDocument_FromJson(TestData_LoadDocJson());
    if(!document) {
        TestData_Free();
        return -1;
    }

    rc = DIDStore_StoreDID(store, document, "");
    if (rc) {
        TestData_Free();
        return -1;
    }

    credential = Credential_FromJson(TestData_LoadVcEmailJson(),
            DIDDocument_GetSubject(document));
    DIDDocument_Destroy(document);
    if(!credential)
        return -1;

    return 0;
}

static int cred_getelem_test_suite_cleanup(void)
{
    Credential_Destroy(credential);
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_cred_get_id",                 test_cred_get_id               },
    { "test_cred_get_type_count",         test_cred_get_type_count       },
    { "test_cred_get_types",              test_cred_get_types            },
    { "test_cred_get_issuer",             test_cred_get_issuer           },
    { "test_cred_get_issuance_date",      test_cred_get_issuance_date    },
    { "test_cred_get_expirate_data",      test_cred_get_expirate_data    },
    { "test_cred_get_property_count",     test_cred_get_property_count   },
    { "test_cred_get_properties",         test_cred_get_properties       },
    { "test_cred_get_property",           test_cred_get_property         },
    { "test_cred_get_proof_method",       test_cred_get_proof_method     },
    { "test_cred_get_proof_type",         test_cred_get_proof_type       },
    { "test_cred_proof_signature",        test_cred_proof_signature      },
    { NULL,                               NULL                           }
};

static CU_SuiteInfo suite[] = {
    {  "credential get elem test",    cred_getelem_test_suite_init,     cred_getelem_test_suite_cleanup,      NULL, NULL, cases },
    {  NULL,                          NULL,                             NULL,                                 NULL, NULL, NULL  }
};


CU_SuiteInfo* cred_getelem_test_suite_info(void)
{
    return suite;
}

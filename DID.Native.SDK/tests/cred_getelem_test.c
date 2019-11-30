#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <CUnit/Basic.h>

#include "loader.h"
#include "ela_did.h"
#include "credential.h"
#include "diddocument.h"

static Credential *credential;
static DID did;

static const char *idstring = "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN";

static void test_cred_get_id(void)
{
    DIDURL *cred_id = Credential_GetId(credential);
    CU_ASSERT_PTR_NOT_NULL_FATAL(cred_id);

    DID *cred_did = DIDURL_GetDid(cred_id);
    CU_ASSERT_STRING_EQUAL_FATAL(DID_GetMethodSpecificString(cred_did), idstring);
}

static void test_cred_get_type_count(void)
{
    ssize_t size = Credential_GetTypeCount(credential);
    CU_ASSERT_EQUAL(size, 2);
}

static void test_cred_get_types(void)
{
    const char *types[2];

    ssize_t size = Credential_GetTypeCount(credential);
    CU_ASSERT_EQUAL(size, 2);

    size = Credential_GetTypes(credential, types, size);
    CU_ASSERT_EQUAL(size, 2);
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
    time_t time = Credential_GetExpirationDate(credential);
    CU_ASSERT_NOT_EQUAL(time, 0);
}

static void test_cred_get_property_count(void)
{
    ssize_t size = Credential_GetPropertyCount(credential);
    CU_ASSERT_EQUAL(size, 4);
}

static void test_cred_get_properties(void)
{
    Property *properties[4];

    ssize_t size = Credential_GetPropertyCount(credential);
    CU_ASSERT_EQUAL(size, 4);

    size = Credential_GetProperties(credential, properties, size);
    CU_ASSERT_EQUAL(size, 4);
    return;
}

static void test_cred_get_property(void)
{
    Property *pro = Credential_GetProperty(credential, "nickname");
    CU_ASSERT_PTR_NOT_NULL(pro);
}

static void test_cred_add_property(void)
{
    ssize_t size = Credential_AddProperty(credential, "phone", "13188673423");
    CU_ASSERT_EQUAL(size, 5);
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
    DIDDocument *doc = DIDDocument_FromJson(global_did_string);
    if(!doc)
        return -1;

    DID_Copy(&did, DIDDocument_GetSubject(doc));
    DIDDocument_Destroy(doc);

    credential = Credential_FromJson(global_cred_string, &did);
    if(!credential)
        return -1;

    return 0;
}

static int cred_getelem_test_suite_cleanup(void)
{
    Credential_Destroy(credential);
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

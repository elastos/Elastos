#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include "constant.h"
#include "loader.h"
#include "ela_did.h"
#include "did.h"
#include "diddocument.h"

static DIDDocument *document;
static DIDURL id;
static DIDURL auth_id;
static DIDURL cred_id;
static DIDURL service_id;

static void test_diddoc_get_subject(void)
{
    DID *did = DIDDocument_GetSubject(document);
    CU_ASSERT_PTR_NOT_NULL(did);
}

static void test_diddoc_get_publickeys_count(void)
{
    ssize_t count = DIDDocument_GetPublicKeysCount(document);
    CU_ASSERT_EQUAL(count, 4);
}

static void test_diddoc_get_publickeys(void)
{
    PublicKey *pks[4];

    ssize_t count = DIDDocument_GetPublicKeysCount(document);
    CU_ASSERT_EQUAL(count, 4);

    count = DIDDocument_GetPublicKeys(document, pks, 4);
    CU_ASSERT_EQUAL(count, 4);
}

static void test_diddoc_get_publickey(void)
{
    PublicKey *pk = DIDDocument_GetPublicKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);
}

static void test_diddoc_select_publickey(void)
{
    PublicKey *pks[4];

    ssize_t count = DIDDocument_SelectPublicKey(document, type, &id, pks, 4);
    CU_ASSERT_EQUAL(count, 1);
}

//authentication
static void test_diddoc_get_authentications_count(void)
{
    ssize_t count = DIDDocument_GetAuthenticationsCount(document);
    CU_ASSERT_EQUAL(count, 3);
}

static void test_diddoc_get_authentications(void)
{
    PublicKey *pks[4];

    ssize_t count = DIDDocument_GetAuthentications(document, pks, 4);
    CU_ASSERT_EQUAL(count, 3);
}

static void test_diddoc_get_authentication(void)
{
    PublicKey *pk = DIDDocument_GetAuthenticationKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);
}

static void test_diddoc_select_authentication(void)
{
    PublicKey *pks[4];

    ssize_t count = DIDDocument_SelectAuthenticationKey(document, type, &id, pks, 4);
    CU_ASSERT_EQUAL(count, 1);
}

//Authorization
static void test_diddoc_get_authorizations_count(void)
{
    ssize_t count = DIDDocument_GetAuthorizationsCount(document);
    CU_ASSERT_EQUAL(count, 1);
}

static void test_diddoc_get_authorizations(void)
{
    PublicKey *pks[4];
    ssize_t count = DIDDocument_GetAuthorizations(document, pks, 4);
    CU_ASSERT_EQUAL(count, 1);
}

static void test_diddoc_get_authorization(void)
{
    PublicKey *pk = DIDDocument_GetAuthorizationKey(document, &auth_id);
    CU_ASSERT_PTR_NOT_NULL(pk);
}

static void test_diddoc_select_authorization(void)
{
    PublicKey *pks[4];

    ssize_t count = DIDDocument_SelectAuthorizationKey(document, type, &auth_id, pks, 4);
    CU_ASSERT_EQUAL(count, 1);
}

//Credential
static void test_diddoc_get_credentials_count(void)
{
    ssize_t count = DIDDocument_GetCredentialsCount(document);
    CU_ASSERT_EQUAL(count, 2);
}

static void test_diddoc_get_credentials(void)
{
    Credential *creds[4];

    ssize_t count = DIDDocument_GetCredentials(document, creds, 4);
    CU_ASSERT_EQUAL(count, 2);
}

static void test_diddoc_get_credential(void)
{
    Credential *cred = DIDDocument_GetCredential(document, &cred_id);
    CU_ASSERT_PTR_NOT_NULL(cred);
}

static void test_diddoc_select_credential(void)
{
    Credential *creds[4];
    ssize_t count = DIDDocument_SelectCredential(document, type, &cred_id, creds, 4);
    CU_ASSERT_EQUAL(count, 1);
}

//service
static void test_diddoc_get_services_count(void)
{
    ssize_t count = DIDDocument_GetServicesCount(document);
    CU_ASSERT_EQUAL(count, 3);
}

static void test_diddoc_get_services(void)
{
    Service *services[4];

    ssize_t count = DIDDocument_GetServices(document, services, 4);
    CU_ASSERT_EQUAL(count, 3);
}

static void test_diddoc_get_service(void)
{
    Service *service = DIDDocument_GetService(document, &service_id);
    CU_ASSERT_PTR_NOT_NULL(service);
}

static void test_diddoc_select_service(void)
{
    Service *services[4];

    ssize_t count = DIDDocument_SelectService(document, service_type, &service_id, services, 4);
    CU_ASSERT_EQUAL(count, 1);
}

//expires
static void test_diddoc_get_expires(void)
{
    time_t time = DIDDocument_GetExpires(document);
    CU_ASSERT_NOT_EQUAL(time, 0);
}

static int diddoc_getelem_test_suite_init(void)
{
    document = DIDDocument_FromJson(global_did_string);
    if(!document)
        return -1;

    strcpy(id.did.idstring, "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
    strcpy(id.fragment, "default");
    strcpy(auth_id.did.idstring, "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
    strcpy(auth_id.fragment, "recovery");
    strcpy(cred_id.did.idstring, "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
    strcpy(cred_id.fragment, "crdential-1");
    strcpy(service_id.did.idstring, "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
    strcpy(service_id.fragment, "openid");

    return 0;
}

static int diddoc_getelem_test_suite_cleanup(void)
{
    DIDDocument_Destroy(document);
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_diddoc_get_subject",                   test_diddoc_get_subject              },
    { "test_diddoc_get_publickeys_count",          test_diddoc_get_publickeys_count     },
    { "test_diddoc_get_publickeys",                test_diddoc_get_publickeys           },
    { "test_diddoc_get_publickey",                 test_diddoc_get_publickey            },
    { "test_diddoc_select_publickey",              test_diddoc_select_publickey         },
    { "test_diddoc_get_authentications_count",     test_diddoc_get_authentications_count},
    { "test_diddoc_get_authentications",           test_diddoc_get_authentications      },
    { "test_diddoc_get_authentication",            test_diddoc_get_authentication       },
    { "test_diddoc_select_authentication",         test_diddoc_select_authentication    },
    { "test_diddoc_get_authorizations_count",      test_diddoc_get_authorizations_count },
    { "test_diddoc_get_authorizations",            test_diddoc_get_authorizations       },
    { "test_diddoc_get_authorization",             test_diddoc_get_authorization        },
    { "test_diddoc_select_authorization",          test_diddoc_select_authorization     },
    { "test_diddoc_get_services_count",            test_diddoc_get_services_count       },
    { "test_diddoc_get_services",                  test_diddoc_get_services             },
    { "test_diddoc_get_service",                   test_diddoc_get_service              },
    { "test_diddoc_select_service",                test_diddoc_select_service           },
    { "test_diddoc_get_expires",                   test_diddoc_get_expires              },
    { NULL,                                        NULL                                 }
};

static CU_SuiteInfo suite[] = {
    {   "diddoc get elem test",    diddoc_getelem_test_suite_init,      diddoc_getelem_test_suite_cleanup,    NULL, NULL, cases },
    {    NULL,                     NULL,                                NULL,                                 NULL, NULL, NULL  }
};


CU_SuiteInfo* diddoc_getelem_test_suite_info(void)
{
    return suite;
}

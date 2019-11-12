#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include "loader.h"
#include "ela_did.h"
#include "did.h"
#include "credential.h"
#include "diddocument.h"

static DIDDocument *document;
static Credential *cred;
static DIDURL id;
static DIDURL service_id;
static DIDURL cred_id;

/*static void test_diddoc_add_publickey(void)
{
    PublicKey *pk;
    int rc;

    rc = DIDDocument_AddPublicKey(document, &id, NULL,
            "H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    pk = DIDDocument_GetPublicKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);

    rc = DIDDocument_RemovePublicKey(document, &id, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    pk = DIDDocument_GetPublicKey(document, &id);
    CU_ASSERT_PTR_NULL(pk);
}*/

static void test_diddoc_add_authentication(void)
{
    PublicKey *pk;
    int rc;

    rc = DIDDocument_AddAuthenticationKey(document, &id, NULL,
            "H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    pk = DIDDocument_GetAuthenticationKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);

    pk = DIDDocument_GetPublicKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);

    rc = DIDDocument_RemoveAuthenticationKey(document, &id);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    pk = DIDDocument_GetAuthenticationKey(document, &id);
    CU_ASSERT_PTR_NULL(pk);

    pk = DIDDocument_GetPublicKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);
}

static void test_diddoc_add_publickey_and_authentication(void)
{
    PublicKey *pk;
    int rc;

    rc = DIDDocument_AddPublicKey(document, &id, NULL,
            "H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    pk = DIDDocument_GetPublicKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);

    rc = DIDDocument_AddAuthenticationKey(document, &id, NULL,
            "H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    pk = DIDDocument_GetAuthenticationKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);

    rc = DIDDocument_RemovePublicKey(document, &id, false);
    CU_ASSERT_EQUAL(rc, -1);

    rc = DIDDocument_RemovePublicKey(document, &id, true);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    pk = DIDDocument_GetAuthenticationKey(document, &id);
    CU_ASSERT_PTR_NULL(pk);

    pk = DIDDocument_GetPublicKey(document, &id);
    CU_ASSERT_PTR_NULL(pk);
}

static void test_diddoc_add_authorization(void)
{
    PublicKey *pk;
    int rc;

    rc = DIDDocument_AddAuthorizationKey(document, &id, NULL,
            "H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    pk = DIDDocument_GetAuthorizationKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);

    pk = DIDDocument_GetPublicKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);

    rc = DIDDocument_RemoveAuthorizationKey(document, &id);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    pk = DIDDocument_GetAuthorizationKey(document, &id);
    CU_ASSERT_PTR_NULL(pk);

    pk = DIDDocument_GetPublicKey(document, &id);
    CU_ASSERT_PTR_NOT_NULL(pk);
}

static void test_diddoc_add_service(void)
{
    Service *service;
    int rc;

    rc = DIDDocument_AddService(document, &service_id,
            "OpenIdConnectVersion1.0Service", "https://openid.example.com/");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    service = DIDDocument_GetService(document, &service_id);
    CU_ASSERT_PTR_NOT_NULL(service);

    rc = DIDDocument_RemoveService(document, &service_id);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    service = DIDDocument_GetService(document, &service_id);
    CU_ASSERT_PTR_NULL(service);
}

static void test_diddoc_add_credential(void)
{
    Credential *credential;
    int rc;

    rc = DIDDocument_AddCredential(document, cred);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    credential = DIDDocument_GetCredential(document, &cred_id);
    CU_ASSERT_PTR_NOT_NULL(credential);

    rc = DIDDocument_RemoveCredential(document, &cred_id);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    credential = DIDDocument_GetCredential(document, &cred_id);
    CU_ASSERT_PTR_NULL(credential);
}

static int diddoc_arelem_test_suite_init(void)
{
    DIDURL *tempid;

    document = DIDDocument_FromJson(global_didbp_string);
    if(!document)
        return -1;

    cred = Credential_FromJson(global_cred_string, &document->did);
    if (!cred) {
        DIDDocument_Destroy(document);
        return -1;
    }

    strcpy(id.did.idstring, "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
    strcpy(id.fragment, "keys3");
    strcpy(service_id.did.idstring, "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");
    strcpy(service_id.fragment, "openid");

    tempid = Credential_GetId(cred);
    if (!tempid) {
        Credential_Destroy(cred);
        DIDDocument_Destroy(document);
        return -1;
    }

    strcpy(cred_id.did.idstring, tempid->did.idstring);
    strcpy(cred_id.fragment, tempid->fragment);

    return 0;
}

static int diddoc_arelem_test_suite_cleanup(void)
{
    DIDDocument_Destroy(document);
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_diddoc_add_publickey_and_authentication", test_diddoc_add_publickey_and_authentication},
    { "test_diddoc_add_authentication",     test_diddoc_add_authentication     },
    { "test_diddoc_add_authorization",      test_diddoc_add_authorization      },
    { "test_diddoc_add_service",            test_diddoc_add_service            },
    { "test_diddoc_add_credential",         test_diddoc_add_credential         },
    { NULL,                                 NULL                               }
};

static CU_SuiteInfo suite[] = {
    { "diddoc add and remove elem test", diddoc_arelem_test_suite_init,  diddoc_arelem_test_suite_cleanup,    NULL, NULL, cases },
    { NULL,                              NULL,                           NULL,                                NULL, NULL, NULL  }
};


CU_SuiteInfo* diddoc_arelem_test_suite_info(void)
{
    return suite;
}

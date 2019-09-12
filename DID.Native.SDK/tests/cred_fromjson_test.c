#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include "loader.h"
#include "credential.h"
#include "diddocument.h"

static Credential *credential;
static DID *did;

static void test_cred_fromjson(void)
{
    credential = Credential_FromJson(global_cred_string, did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(credential);
}

static int cred_fromjson_test_suite_init(void)
{
    DIDDocument *doc = DIDDocument_FromJson(global_did_string);
    if(!doc)
        return -1;

    did = (DID*)calloc(1, sizeof(DID));
    if (!did) {
        DIDDocument_Destroy(doc);
        return -1;
    }

    strcpy(did->idstring, doc->did.idstring);

    DIDDocument_Destroy(doc);

    return 0;
}

static int cred_fromjson_test_suite_cleanup(void)
{
    Credential_Destroy(credential);
    DID_Destroy(did);
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_cred_fromjson",            test_cred_fromjson    },
    { NULL,                            NULL                  }
};

static CU_SuiteInfo suite[] = {
    { "credential from json test",   cred_fromjson_test_suite_init,   cred_fromjson_test_suite_cleanup,     NULL, NULL, cases },
    {  NULL,                         NULL,                            NULL,                                 NULL, NULL, NULL  }
};


CU_SuiteInfo* cred_fromjson_test_suite_info(void)
{
    return suite;
}

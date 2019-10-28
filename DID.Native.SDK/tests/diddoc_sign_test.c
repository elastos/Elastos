#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <crystal.h>
#include <CUnit/Basic.h>
#include <limits.h>

#include "loader.h"
#include "ela_did.h"
#include "didrequest.h"
#include "didstore.h"
#include "diddocument.h"

#define SIGNATURE_BYTES 65

static DIDDocument *document;
static DID *did;
static const char *mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";
static const char *data = "abcdefghijklmnopqrstuvwxyz";
static const char *storepass = "123456";
static char signature[SIGNATURE_BYTES * 2];

static void test_diddoc_sign(void)
{
    int rc;
    DIDURL *signkey;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    rc = DIDDocument_Sign(document, signkey, storepass, signature, 1,
            (unsigned char*)data, strlen(data));
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_diddoc_verify(void)
{
    int rc;
    DIDURL *signkey;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    rc = DIDDocument_Verify(document, signkey, signature, 1,
             (unsigned char*)data, strlen(data));
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static int diddoc_sign_test_suite_init(void)
{
    char current_path[PATH_MAX];
    int rc;

    if(!getcwd(current_path, PATH_MAX)) {
        printf("\nCan't get current dir.");
        return -1;
    }

    strcat(current_path, "/newdid");
    if (DIDStore_Open(current_path) == -1)
        return -1;

    rc = DIDStore_InitPrivateIdentity(mnemonic, "", storepass, 0);
    if (rc < 0)
        return -1;

    document = DIDStore_NewDID(storepass, "littlefish");
    if(!document)
        return -1;

    did = DIDDocument_GetSubject(document);
    if (!did)
        return -1;

    return 0;
}

static int diddoc_sign_test_suite_cleanup(void)
{
    DIDStore_DeleteDID(did);
    DIDDocument_Destroy(document);;
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_diddoc_sign",        test_diddoc_sign      },
    {   "test_diddoc_verify",      test_diddoc_verify    },
    {   NULL,                      NULL                  }
};

static CU_SuiteInfo suite[] = {
    {   "diddoc sign test",    diddoc_sign_test_suite_init,   diddoc_sign_test_suite_cleanup,   NULL, NULL, cases },
    {    NULL,                 NULL,                          NULL,                             NULL, NULL, NULL  }
};

CU_SuiteInfo* diddoc_sign_test_suite_info(void)
{
    return suite;
}
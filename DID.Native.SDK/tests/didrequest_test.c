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

static DIDDocument *document;
static DID *did;
static const char *mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";

static void test_didrequest_publishdid(void)
{
    int rc;
    DIDURL *signkey;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    rc = DIDREQ_PublishDID(document, signkey, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didrequest_updatedid(void)
{
    int rc;
    DIDURL *signkey;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    rc = DIDREQ_UpdateDID(document, signkey, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static void test_didrequest_deactivatedid(void)
{
    int rc;
    DIDURL *signkey;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    if (!signkey) {
        printf("get signkey failed.\n");
        return;
    }

    rc = DIDREQ_DeactivateDID(&(document->did), signkey, "");
    CU_ASSERT_NOT_EQUAL(rc, -1);
}

static int didrequest_test_suite_init(void)
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

    rc = DIDStore_InitPrivateIdentity(mnemonic, "", "", 0);
    if (rc < 0)
        return -1;

    document = DIDStore_NewDID("", "littlefish");
    if(!document)
        return -1;

    did = DIDDocument_GetSubject(document);
    if (!did)
        return -1;

    return 0;
}

static int didrequest_test_suite_cleanup(void)
{
    DIDStore_DeleteDID(did);
    DIDDocument_Destroy(document);;
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_didrequest_publishdid",        test_didrequest_publishdid      },
    {   "test_didrequest_updatedid",         test_didrequest_updatedid       },
    {   "test_didrequest_deactivatedid",     test_didrequest_deactivatedid   },
    {   NULL,                                 NULL                           }
};

static CU_SuiteInfo suite[] = {
    {   "didrequest test",     didrequest_test_suite_init,    didrequest_test_suite_cleanup,   NULL, NULL, cases },
    {    NULL,                 NULL,                          NULL,                            NULL, NULL, NULL  }
};

CU_SuiteInfo* didrequest_test_suite_info(void)
{
    return suite;
}
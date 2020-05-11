#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <crystal.h>
#include <CUnit/Basic.h>
#include <limits.h>

#include "constant.h"
#include "loader.h"
#include "ela_did.h"
#include "did.h"

#define MAX_DOC_SIGN                    128

static int didcount = 0;

typedef struct DIDs {
    DID dids[10];
    int index;
} DIDs;

static int load_restore(DIDs *dids, const char *path)
{
    FILE *fd;
    char *find;
    char data[128];
    int i = 0;

    if (!path || !*path)
        return -1;

    fd = fopen(path , "r");
    if (!fd)
        return -1;

    while(!feof(fd)) {
        if (!fgets(data, sizeof(data), fd))
            return -1;

        find = strchr(data, '\n');
        if (find)
            *find = '\0';

        DID *did = DID_FromString(data);
        if (!did)
            break;

        DID_Copy(&(dids->dids[dids->index++]), did);
        DID_Destroy(did);
    }

    return 0;
}

static bool contain_did(DIDs *dids, DID *did)
{
    if (!dids || !did)
        return -1;

    for (int i = 0; i < dids->index; i++) {
        if (DID_Equals(&(dids->dids[i]), did))
            return true;
    }
    return false;
}

static int get_did(DID *did, void *context)
{
    DIDs *dids = (DIDs*)context;

    char alias[ELA_MAX_ALIAS_LEN];
    DIDDocument *doc = NULL;
    int rc;

    if (!did)
        return 0;

    if (dids->index >= 10)
        return -1;

    DID_Copy(&(dids->dids[dids->index++]), did);
    return 0;
}

static DIDDocument* merge_to_localcopy(DIDDocument *chaincopy, DIDDocument *localcopy)
{
    if (!chaincopy && !localcopy)
        return NULL;

    if (!chaincopy)
        return chaincopy;

    return localcopy;
}

static DIDDocument* merge_to_chaincopy(DIDDocument *chaincopy, DIDDocument *localcopy)
{
    if (!chaincopy && !localcopy)
        return NULL;

    if (chaincopy)
        return chaincopy;

    return NULL;
}

static void test_idchain_restore(void)
{
    int rc;
    char _path[PATH_MAX];
    const char *path, *mnemonic;
    DIDStore *store;
    DIDs dids;
    DIDs restore_dids;

    path = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(false, path);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    rc = DIDStore_InitPrivateIdentity(store, storepass, TestData_LoadRestoreMnemonic(),
        "secret", language, true);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    printf("\nSynchronizing from IDChain...");
    rc = DIDStore_Synchronize(store, storepass, merge_to_localcopy);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    printf("OK!\n");

    memset(&dids, 0, sizeof(DIDs));
    rc = DIDStore_ListDIDs(store, 0, get_did, (void*)&dids);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(dids.index, 5);

    path = get_path(_path, "dids.restore");
    CU_ASSERT_PTR_NOT_NULL_FATAL(path);

    memset(&restore_dids, 0, sizeof(DIDs));
    rc = load_restore(&restore_dids, path);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    CU_ASSERT_EQUAL_FATAL(dids.index, restore_dids.index);

    for(int i = 0; i < restore_dids.index; i++) {
        DID *did = &restore_dids.dids[i];
        bool iscontain = contain_did(&dids, did);
        CU_ASSERT_TRUE(iscontain);

        DIDDocument *doc = DIDStore_LoadDID(store, did);
        CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
        CU_ASSERT_TRUE_FATAL(DID_Equals(did, DIDDocument_GetSubject(doc)));
        CU_ASSERT_EQUAL_FATAL(4, DIDDocument_GetCredentialCount(doc));

        Credential *creds[4];
        ssize_t size = DIDDocument_GetCredentials(doc, creds, 4);
        CU_ASSERT_EQUAL_FATAL(4, size);

        for (int j = 0; j < size; j++) {
            Credential *cred = creds[j];
            CU_ASSERT_PTR_NOT_NULL_FATAL(cred);
            CU_ASSERT_TRUE_FATAL(DID_Equals(did, Credential_GetOwner(cred)));
        }
    }

    TestData_Free();
}

static void test_sync_with_localmodification1(void)
{
    int rc;
    char _path[PATH_MAX], modified_signature[MAX_DOC_SIGN];
    const char *path, *mnemonic;
    DIDStore *store;
    DIDs dids;

    path = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(false, path);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    rc = DIDStore_InitPrivateIdentity(store, storepass, TestData_LoadRestoreMnemonic(),
        "secret", language, true);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    printf("\nSynchronizing from IDChain...");
    rc = DIDStore_Synchronize(store, storepass, merge_to_localcopy);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    printf("OK!\n");

    memset(&dids, 0, sizeof(DIDs));
    rc = DIDStore_ListDIDs(store, 0, get_did, (void*)&dids);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(dids.index, 5);

    DID *modified_did = &dids.dids[0];
    DIDDocument *modified_doc = DIDStore_LoadDID(store, modified_did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(modified_doc);

    DIDDocumentBuilder *builder = DIDDocument_Edit(modified_doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(builder);
    DIDDocument_Destroy(modified_doc);

    DIDURL *serviceid = DIDURL_NewByDid(modified_did, "test1");
    CU_ASSERT_PTR_NOT_NULL_FATAL(serviceid);

    rc = DIDDocumentBuilder_AddService(builder, serviceid, "TestType", "http://test.com/");
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    DIDURL_Destroy(serviceid);

    modified_doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(modified_doc);

    rc = DIDStore_StoreDID(store, modified_doc, NULL);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    strcpy(modified_signature, DIDDocument_GetProofSignature(modified_doc));
    DIDDocument_Destroy(modified_doc);

    printf("Synchronizing again from IDChain...");
    rc = DIDStore_Synchronize(store, storepass, merge_to_localcopy);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    memset(&dids, 0, sizeof(DIDs));
    rc = DIDStore_ListDIDs(store, 0, get_did, (void*)&dids);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(dids.index, 5);

    for(int i = 0; i < dids.index; i++) {
        DID *did = &dids.dids[i];
        bool iscontain = contain_did(&dids, did);
        CU_ASSERT_TRUE(iscontain);

        DIDDocument *doc = DIDStore_LoadDID(store, did);
        CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
        CU_ASSERT_TRUE_FATAL(DID_Equals(did, DIDDocument_GetSubject(doc)));
        CU_ASSERT_EQUAL_FATAL(4, DIDDocument_GetCredentialCount(doc));

        Credential *creds[4];
        ssize_t size = DIDDocument_GetCredentials(doc, creds, 4);
        CU_ASSERT_EQUAL_FATAL(4, size);

        for (int j = 0; j < size; j++) {
            Credential *cred = creds[j];
            CU_ASSERT_PTR_NOT_NULL_FATAL(cred);
            CU_ASSERT_TRUE_FATAL(DID_Equals(did, Credential_GetOwner(cred)));
        }
    }

    modified_doc = DIDStore_LoadDID(store, modified_did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(modified_doc);
    CU_ASSERT_STRING_EQUAL(modified_signature, DIDDocument_GetProofSignature(modified_doc));
    DIDDocument_Destroy(modified_doc);

    TestData_Free();
}


static void test_sync_with_localmodification2(void)
{
    int rc;
    char _path[PATH_MAX], origin_signature[MAX_DOC_SIGN];
    const char *path, *mnemonic;
    DIDStore *store;
    DIDs dids;

    path = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(false, path);
    CU_ASSERT_PTR_NOT_NULL_FATAL(store);

    rc = DIDStore_InitPrivateIdentity(store, storepass, TestData_LoadRestoreMnemonic(),
        "secret", language, true);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    printf("\nSynchronizing from IDChain...");
    rc = DIDStore_Synchronize(store, storepass, merge_to_localcopy);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    printf("OK!\n");

    memset(&dids, 0, sizeof(DIDs));
    rc = DIDStore_ListDIDs(store, 0, get_did, (void*)&dids);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(dids.index, 5);

    DID *modified_did = &dids.dids[0];
    DIDDocument *modified_doc = DIDStore_LoadDID(store, modified_did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(modified_doc);
    strcpy(origin_signature, DIDDocument_GetProofSignature(modified_doc));

    DIDDocumentBuilder *builder = DIDDocument_Edit(modified_doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(builder);
    DIDDocument_Destroy(modified_doc);

    DIDURL *serviceid = DIDURL_NewByDid(modified_did, "test1");
    CU_ASSERT_PTR_NOT_NULL_FATAL(serviceid);

    rc = DIDDocumentBuilder_AddService(builder, serviceid, "TestType", "http://test.com/");
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    DIDURL_Destroy(serviceid);

    modified_doc = DIDDocumentBuilder_Seal(builder, storepass);
    CU_ASSERT_PTR_NOT_NULL_FATAL(modified_doc);

    rc = DIDStore_StoreDID(store, modified_doc, NULL);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    DIDDocument_Destroy(modified_doc);

    printf("Synchronizing again from IDChain...");
    rc = DIDStore_Synchronize(store, storepass, merge_to_chaincopy);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);

    memset(&dids, 0, sizeof(DIDs));
    rc = DIDStore_ListDIDs(store, 0, get_did, (void*)&dids);
    CU_ASSERT_NOT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(dids.index, 5);

    for(int i = 0; i < dids.index; i++) {
        DID *did = &dids.dids[i];
        bool iscontain = contain_did(&dids, did);
        CU_ASSERT_TRUE(iscontain);

        DIDDocument *doc = DIDStore_LoadDID(store, did);
        CU_ASSERT_PTR_NOT_NULL_FATAL(doc);
        CU_ASSERT_TRUE_FATAL(DID_Equals(did, DIDDocument_GetSubject(doc)));
        CU_ASSERT_EQUAL_FATAL(4, DIDDocument_GetCredentialCount(doc));

        Credential *creds[4];
        ssize_t size = DIDDocument_GetCredentials(doc, creds, 4);
        CU_ASSERT_EQUAL_FATAL(4, size);

        for (int j = 0; j < size; j++) {
            Credential *cred = creds[j];
            CU_ASSERT_PTR_NOT_NULL_FATAL(cred);
            CU_ASSERT_TRUE_FATAL(DID_Equals(did, Credential_GetOwner(cred)));
        }
    }

    modified_doc = DIDStore_LoadDID(store, modified_did);
    CU_ASSERT_PTR_NOT_NULL_FATAL(modified_doc);
    CU_ASSERT_STRING_EQUAL(origin_signature, DIDDocument_GetProofSignature(modified_doc));
    DIDDocument_Destroy(modified_doc);

    TestData_Free();
}

static int idchain_restore_test_suite_init(void)
{
    return 0;
}

static int idchain_restore_test_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_idchain_restore",              test_idchain_restore              },
    {   "test_sync_with_localmodification1", test_sync_with_localmodification1 },
    {   "test_sync_with_localmodification2", test_sync_with_localmodification2 },
    {   NULL,                                NULL                              }
};

static CU_SuiteInfo suite[] = {
    { "id chain restore test", idchain_restore_test_suite_init, idchain_restore_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                   NULL,                            NULL,                               NULL, NULL, NULL  }
};

CU_SuiteInfo* idchain_restore_test_suite_info(void)
{
    return suite;
}
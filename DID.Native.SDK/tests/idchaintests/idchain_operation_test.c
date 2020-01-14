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
#include "didmeta.h"

static DIDDocument *document;
static DIDStore *store;

static void test_idchain_publishdid(void)
{
    char *txid;
    DIDURL *signkey;
    DIDDocument *doc = NULL, *updatedoc = NULL;
    char createTxid[ELA_MAX_TXID_LEN], updateTxid[ELA_MAX_TXID_LEN];
    const char *data;
    DID *did;
    int i = 0;

    signkey = DIDDocument_GetDefaultPublicKey(document);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    did = DIDDocument_GetSubject(document);
    CU_ASSERT_PTR_NOT_NULL(did);

    txid = (char *)DIDStore_PublishDID(store, did, signkey, storepass);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    free(txid);

    printf("\n#### begin to resolve(create) [%s]", did->idstring);
    while(!doc) {
        doc = DIDStore_ResolveDID(store, did, true);
        if (!doc) {
            ++i;
            printf(".");
            sleep(30);
        }
        else {
            DIDDocument_GetTxid(doc, createTxid, sizeof(createTxid));
            data = DIDDocument_ToJson(doc, 0, 0);
            printf("\n#### document resolved: time = %d.\n", ++i);
            printf("#### did: %s, transaction id: %s\n", did->idstring, createTxid);
            printf("#### document: %s\n", data);
            free((char*)data);
        }

    }
    printf("#### end resolve\n");

    printf("\n#### begin to update did\n");
    signkey = DIDDocument_GetDefaultPublicKey(doc);
    CU_ASSERT_PTR_NOT_NULL_FATAL(signkey);

    txid = (char *)DIDStore_PublishDID(store, did, signkey, storepass);
    CU_ASSERT_NOT_EQUAL_FATAL(txid, NULL);
    free(txid);

    printf("#### begin to resolve(update) [%s]", did->idstring);
    while(!updatedoc || !strcmp(createTxid, updateTxid)) {
        updatedoc = DIDStore_ResolveDID(store, did, true);
        if (!updatedoc) {
            ++i;
            printf(".");
            sleep(30);
            continue;
        }
        else {
            DIDDocument_GetTxid(updatedoc, updateTxid, sizeof(updateTxid));
            data = DIDDocument_ToJson(updatedoc, 0, 0);
            printf(".");
            continue;
        }
    }
    printf("\n#### did: %s, transaction id: %s\n", did->idstring, updateTxid);
    printf("#### document: %s\n", data);
    printf("#### end resolve\n");
    free((char*)data);

    DIDDocument_Destroy(doc);
    DIDDocument_Destroy(updatedoc);
}

static int idchain_operation_test_suite_init(void)
{
    int rc;
    char _path[PATH_MAX];
    const char *storePath, *mnemonic;

    storePath = get_store_path(_path, "/idchain");
    store = TestData_SetupStore(storePath);
    if (!store)
        return -1;

    mnemonic = Mnemonic_Generate(0);
    printf("\n#### mnemonic: %s", mnemonic);
    rc = DIDStore_InitPrivateIdentity(store, mnemonic, "", storepass, 0, true);
    Mnemonic_free((char*)mnemonic);
    if (rc < 0)
        return -1;

    document = DIDStore_NewDID(store, storepass, "littlefish");
    if (!document)
        return -1;

    return 0;
}

static int idchain_operation_test_suite_cleanup(void)
{
    DIDDocument_Destroy(document);
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_idchain_publishdid",           test_idchain_publishdid      },
    {   NULL,                                NULL                         }
};

static CU_SuiteInfo suite[] = {
    { "id chain operateion test", idchain_operation_test_suite_init, idchain_operation_test_suite_cleanup, NULL, NULL, cases },
    {  NULL,                      NULL,                              NULL,                                 NULL, NULL, NULL  }
};

CU_SuiteInfo* idchain_operation_test_suite_info(void)
{
    return suite;
}
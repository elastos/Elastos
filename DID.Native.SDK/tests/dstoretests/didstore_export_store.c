#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <CUnit/Basic.h>
#include <crystal.h>
#include <limits.h>

#include "constant.h"
#include "loader.h"
#include "ela_did.h"
#include "diddocument.h"
#include "didtest_adapter.h"
#include "didstore.h"

static const char *alias = "littlefish";
static const char *password = "passwd";

static int get_did(DID *did, void *context)
{
    DID *d = (DID*)context;

    if (!did)
        return 0;

    if (strlen(d->idstring) == 0)
        strcpy(d->idstring, did->idstring);

    return 0;
}

static char *get_tmp_file(char *path, const char *filename)
{
    assert(filename && *filename);

    return get_file_path(path, PATH_MAX, 7, "..", PATH_STEP, "etc", PATH_STEP,
           "tmp", PATH_STEP, filename);
}

static void test_didstore_export_import_did(void)
{
    DID did;
    char *file;
    char _path[PATH_MAX], _path2[PATH_MAX], command[512];
    const char *path, *path2;
    int rc;

    DIDStore *store = TestData_SetupTestStore(false);
    CU_ASSERT_PTR_NOT_NULL(store);

    memset(&did, 0, sizeof(did));
    rc = DIDStore_ListDIDs(store, 0, get_did, (void*)&did);
    CU_ASSERT_NOT_EQUAL(rc, -1);

    file = get_tmp_file(_path, "didexport.json");
    CU_ASSERT_PTR_NOT_NULL(file);

    rc = DIDStore_ExportDID(store, password, &did, file, "1234");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    //create new store
    DIDAdapter *adapter = TestData_GetAdapter(true);
    CU_ASSERT_PTR_NOT_NULL(adapter);

    path = get_store_path(_path2, "/restore");
    CU_ASSERT_PTR_NOT_NULL(path);
    delete_file(path);

    DIDStore *store2 = DIDStore_Open(path, adapter);
    CU_ASSERT_PTR_NOT_NULL(store2);

    rc = DIDStore_ImportDID(store2, password, file, "1234");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    delete_file(file);

    path = get_file_path(_path, PATH_MAX, 5, store->root, PATH_STEP, DID_DIR,
            PATH_STEP, did.idstring);
    CU_ASSERT_TRUE_FATAL(dir_exist(path));

    path2 = get_file_path(_path2, PATH_MAX, 5, store2->root, PATH_STEP, DID_DIR,
            PATH_STEP, did.idstring);
    CU_ASSERT_TRUE_FATAL(dir_exist(path));

    // to diff directory
    //sprintf(command, "diff -r %s %s", path, path2);
    //CU_ASSERT_EQUAL(system(command), 0);

    DIDStore_Close(store2);

    TestData_Free();
}

static void test_didstore_export_import_privateidentity(void)
{
    DID did;
    char *file;
    char _path[PATH_MAX], _path2[PATH_MAX], command[512];
    const char *path, *path2;
    int rc;

    DIDStore *store = TestData_SetupTestStore(false);
    CU_ASSERT_PTR_NOT_NULL(store);

    file = get_tmp_file(_path, "idexport.json");
    CU_ASSERT_PTR_NOT_NULL(file);

    rc = DIDStore_ExportPrivateIdentity(store, password, file, "1234");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    //create new store
    DIDAdapter *adapter = TestData_GetAdapter(true);
    CU_ASSERT_PTR_NOT_NULL(adapter);

    path = get_store_path(_path2, "/restore");
    CU_ASSERT_PTR_NOT_NULL(path);

    delete_file(path);

    DIDStore *store2 = DIDStore_Open(path, adapter);
    CU_ASSERT_PTR_NOT_NULL(store2);

    rc = DIDStore_ImportPrivateIdentity(store2, password, file, "1234");
    CU_ASSERT_NOT_EQUAL(rc, -1);
    delete_file(file);

    path = get_file_path(_path, PATH_MAX, 3, store->root, PATH_STEP, PRIVATE_DIR);
    CU_ASSERT_TRUE_FATAL(dir_exist(path));

    path2 = get_file_path(_path2, PATH_MAX, 3, store2->root, PATH_STEP, PRIVATE_DIR);
    CU_ASSERT_TRUE_FATAL(dir_exist(path));

    // to diff directory
    //sprintf(command, "diff -r %s %s", path, path2);
    //CU_ASSERT_EQUAL(system(command), 0);

    DIDStore_Close(store2);
    TestData_Free();
}

static void test_didstore_export_import_store(void)
{
    DID did;
    char *file;
    char _path[PATH_MAX], _path2[PATH_MAX], command[512];
    const char *path, *path2;
    int rc;

    DIDStore *store = TestData_SetupTestStore(false);
    CU_ASSERT_PTR_NOT_NULL(store);

    file = get_tmp_file(_path, "storeexport.zip");
    CU_ASSERT_PTR_NOT_NULL(file);

    rc = DIDStore_ExportStore(store, password, file, "1234");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    //create new store
    DIDAdapter *adapter = TestData_GetAdapter(true);
    CU_ASSERT_PTR_NOT_NULL(adapter);

    path = get_store_path(_path2, "/restore");
    CU_ASSERT_PTR_NOT_NULL(path);

    delete_file(path);

    DIDStore *store2 = DIDStore_Open(path, adapter);
    CU_ASSERT_PTR_NOT_NULL(store2);

    rc = DIDStore_ImportStore(store2, password, file, "1234");
    CU_ASSERT_NOT_EQUAL(rc, -1);

    path = get_file_path(_path, PATH_MAX, 3, store->root, PATH_STEP, DID_DIR);
    CU_ASSERT_TRUE_FATAL(dir_exist(path));

    path2 = get_file_path(_path2, PATH_MAX, 3, store2->root, PATH_STEP, DID_DIR);
    CU_ASSERT_TRUE_FATAL(dir_exist(path));

    // to diff directory
    //sprintf(command, "diff -r %s %s", path, path2);
    //CU_ASSERT_EQUAL(system(command), 0);

    path = get_file_path(_path, PATH_MAX, 3, store->root, PATH_STEP, PRIVATE_DIR);
    CU_ASSERT_TRUE_FATAL(dir_exist(path));

    path2 = get_file_path(_path2, PATH_MAX, 3, store2->root, PATH_STEP, PRIVATE_DIR);
    CU_ASSERT_TRUE_FATAL(dir_exist(path));

    // to diff directory
    //sprintf(command, "diff -r %s %s", path, path2);
    //CU_ASSERT_EQUAL(system(command), 0);

    DIDStore_Close(store2);
    TestData_Free();
}

static int didstore_export_store_test_suite_init(void)
{
    return 0;
}

static int didstore_export_store_test_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {  "test_didstore_export_import_did",              test_didstore_export_import_did              },
    {  "test_didstore_export_import_privateidentity",  test_didstore_export_import_privateidentity  },
    {  "test_didstore_export_import_store",            test_didstore_export_import_store            },
    {  NULL,                                           NULL                                         }
};

static CU_SuiteInfo suite[] = {
    {  "didstore export store test",  didstore_export_store_test_suite_init,  didstore_export_store_test_suite_cleanup,   NULL, NULL, cases },
    {  NULL,                          NULL,                                   NULL,                                  NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_export_store_test_suite_info(void)
{
    return suite;
}

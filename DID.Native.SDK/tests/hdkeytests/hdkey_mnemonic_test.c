#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include <limits.h>

#include "ela_did.h"
#include "HDkey.h"
#include "constant.h"
#include "loader.h"

static DIDStore *store;

static void test_build_wordlist(void)
{
    int rc, size;

    for (int i = 0; i < 6; i++) {
        char wmnemonic[256];
        const char *mnemonic = Mnemonic_Generate(i);
        CU_ASSERT_PTR_NOT_NULL(mnemonic);
        CU_ASSERT_TRUE(Mnemonic_IsValid(mnemonic, i));

        rc = DIDStore_InitPrivateIdentity(store, mnemonic, "", storepass, i, true);
        CU_ASSERT_NOT_EQUAL(rc, -1);

        strcpy(wmnemonic, mnemonic);
        strcat(wmnemonic, "z");
        CU_ASSERT_FALSE(Mnemonic_IsValid(wmnemonic, i));

        Mnemonic_free((void*)mnemonic);
    }
}

static int hdkey_mnemonic_test_suite_init(void)
{
    char _path[PATH_MAX];
    const char *storePath;

    storePath = get_store_path(_path, "/servet");
    store = TestData_SetupStore(storePath);
    if (!store)
        return -1;

    return 0;
}

static int hdkey_mnemonic_test_suite_cleanup(void)
{
    TestData_Free();
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_build_wordlist",     test_build_wordlist        },
    {   NULL,                      NULL                       }
};

static CU_SuiteInfo suite[] = {
    { "hdkey mnemonic test",  hdkey_mnemonic_test_suite_init,  hdkey_mnemonic_test_suite_cleanup, NULL, NULL, cases },
    { NULL,                   NULL,                            NULL,                              NULL, NULL, NULL  }
};

CU_SuiteInfo* hdkey_mnemonic_test_suite_info(void)
{
    return suite;
}
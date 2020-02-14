#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "HDkey.h"
#include "crypto.h"

static void test_rootkey_with_diff_method(void)
{
    uint8_t _seed[SEED_BYTES];
    uint8_t *seed;
    uint8_t ExtendedKey[EXTENDEDKEY_BYTES], _ExtendedKey[EXTENDEDKEY_BYTES];
    ssize_t size;
    HDKey _hdkey, *hdkey;
    DerivedKey _derivedkey, *derivedkey;

    const char *mnemonic = "pact reject sick voyage foster fence warm luggage cabbage any subject carbon";
    const char *ExtendedkeyBase = "xprv9s21ZrQH143K4biiQbUq8369meTb1R8KnstYFAKtfwk3vF8uvFd1EC2s49bMQsbdbmdJxUWRkuC48CXPutFfynYFVGnoeq8LJZhfd9QjvUt";
    const char *passphrase = "helloworld";

    size = base58_decode(ExtendedKey, ExtendedkeyBase);
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);

    hdkey = HDKey_FromMnemonic(mnemonic, passphrase, 0, &_hdkey);
    CU_ASSERT_PTR_NOT_NULL(hdkey);

    size = HDKey_Serialize(hdkey, _ExtendedKey, sizeof(_ExtendedKey));
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);
    CU_ASSERT_NSTRING_EQUAL(ExtendedKey, _ExtendedKey, EXTENDEDKEY_BYTES - 4);
}

static int hdkey_rootkey_test_suite_init(void)
{
    return 0;
}

static int hdkey_rootkey_test_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    { "test_rootkey_with_diff_method",     test_rootkey_with_diff_method   },
    {  NULL,                               NULL                            }
};

static CU_SuiteInfo suite[] = {
    { "hdkey rootkey test",  hdkey_rootkey_test_suite_init,  hdkey_rootkey_test_suite_cleanup, NULL, NULL, cases },
    { NULL,                  NULL,                           NULL,                             NULL, NULL, NULL  }
};

CU_SuiteInfo* hdkey_rootkey_test_suite_info(void)
{
    return suite;
}
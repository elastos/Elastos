#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "HDkey.h"
#include "crypto.h"

#define MAX_PUBLICKEY_BASE58 64

static const char *mnemonic = "pact reject sick voyage foster fence warm luggage cabbage any subject carbon";
static const char *passphrase = "helloworld";

static void test_rootkey_with_diff_method(void)
{
    uint8_t _seed[SEED_BYTES], extendedkey[EXTENDEDKEY_BYTES], _extendedkey[EXTENDEDKEY_BYTES];
    uint8_t *seed;
    ssize_t size;
    HDKey _hdkey, *hdkey;
    DerivedKey _derivedkey, *derivedkey;

    const char *extendedprvbase58 = "xprv9s21ZrQH143K4biiQbUq8369meTb1R8KnstYFAKtfwk3vF8uvFd1EC2s49bMQsbdbmdJxUWRkuC48CXPutFfynYFVGnoeq8LJZhfd9QjvUt";
    const char *extendedpubbase58 = "xpub6BmohzsffkuPQHqRNqksqvnef6c3wKarsRAmBjRHZgkLrT91xzH3HnkkJv48oursb6CxdzwuDecozwCXF5t9ropBqpPVz4hw2foivZxsmVs";

    hdkey = HDKey_FromMnemonic(mnemonic, passphrase, "english", &_hdkey);
    CU_ASSERT_PTR_NOT_NULL(hdkey);

    size = HDKey_SerializePrv(hdkey, _extendedkey, sizeof(_extendedkey));
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);

    size = base58_decode(extendedkey, sizeof(extendedkey), extendedprvbase58);
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);
    CU_ASSERT_NSTRING_EQUAL(extendedkey, _extendedkey, EXTENDEDKEY_BYTES - 4);

    size = HDKey_SerializePub(hdkey, _extendedkey, sizeof(_extendedkey));
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);

    size = base58_decode(extendedkey, sizeof(extendedkey), extendedpubbase58);
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);
    CU_ASSERT_NSTRING_EQUAL(extendedkey, _extendedkey, EXTENDEDKEY_BYTES - 4);
}

static void test_derive_publiconly(void)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES], _extendedkey[EXTENDEDKEY_BYTES];
    ssize_t size;
    HDKey _prvhdkey, *prvhdkey;
    HDKey _pubhdkey, *pubhdkey;
    DerivedKey _prvderivedkey, *prvderivedkey;
    DerivedKey _pubderivedkey, *pubderivedkey;
    char prvbase58[MAX_PUBLICKEY_BASE58], pubbse58[MAX_PUBLICKEY_BASE58];

    prvhdkey = HDKey_FromMnemonic(mnemonic, passphrase, "english", &_prvhdkey);
    CU_ASSERT_PTR_NOT_NULL(prvhdkey);

    size = HDKey_SerializePub(prvhdkey, extendedkey, sizeof(extendedkey));
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);

    pubhdkey = HDKey_FromExtendedKey(extendedkey, size, &_pubhdkey);
    CU_ASSERT_PTR_NOT_NULL(pubhdkey);

    for (int i = 0; i < 100; i++) {
        prvderivedkey = HDKey_GetDerivedKey(prvhdkey, i, &_pubderivedkey);
        pubderivedkey = HDKey_GetDerivedKey(prvhdkey, i, &_pubderivedkey);

        CU_ASSERT_STRING_EQUAL(DerivedKey_GetPublicKeyBase58(prvderivedkey, prvbase58, sizeof(prvbase58)),
                DerivedKey_GetPublicKeyBase58(pubderivedkey, pubbse58, sizeof(pubbse58)));
        CU_ASSERT_STRING_EQUAL(DerivedKey_GetAddress(prvderivedkey),
                DerivedKey_GetAddress(pubderivedkey));
    }
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
    { "test_derive_publiconly",            test_derive_publiconly          },
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
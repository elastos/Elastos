#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "HDkey.h"
#include "crypto.h"
#include "ela_did.h"

#define MAX_PUBLICKEY_BASE58           64
#define HARDENED                       0x80000000

static const char *mnemonic = "pact reject sick voyage foster fence warm luggage cabbage any subject carbon";
static const char *passphrase = "helloworld";

static void test_rootkey_with_diff_method(void)
{
    uint8_t _seed[SEED_BYTES], extendedkey[EXTENDEDKEY_BYTES], _extendedkey[EXTENDEDKEY_BYTES];
    uint8_t *seed;
    ssize_t size;
    HDKey _hdkey, *hdkey;
    HDKey _prederivedkey, *prederivedkey;

    //extended private key: get from seed/mnemonic
    const char *extendedprvbase58 = "xprv9s21ZrQH143K4biiQbUq8369meTb1R8KnstYFAKtfwk3vF8uvFd1EC2s49bMQsbdbmdJxUWRkuC48CXPutFfynYFVGnoeq8LJZhfd9QjvUt";
    //prederive extended public key: get from 44'/0'/0', this key is designed to generate hdkey address without storepass.
    const char *extendedpubbase58 = "xpub6BmohzsffkuPQHqRNqksqvnef6c3wKarsRAmBjRHZgkLrT91xzH3HnkkJv48oursb6CxdzwuDecozwCXF5t9ropBqpPVz4hw2foivZxsmVs";

    hdkey = HDKey_FromMnemonic(mnemonic, passphrase, "english", &_hdkey);
    CU_ASSERT_PTR_NOT_NULL(hdkey);

    size = HDKey_SerializePrv(hdkey, _extendedkey, sizeof(_extendedkey));
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);

    size = base58_decode(extendedkey, sizeof(extendedkey), extendedprvbase58);
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);
    CU_ASSERT_NSTRING_EQUAL(extendedkey, _extendedkey, EXTENDEDKEY_BYTES);

    prederivedkey = HDKey_GetDerivedKey(hdkey, &_prederivedkey, 3, 44 | HARDENED,
           0 | HARDENED, 0 | HARDENED);
    CU_ASSERT_PTR_NOT_NULL(prederivedkey);

    size = HDKey_SerializePub(prederivedkey, _extendedkey, sizeof(_extendedkey));
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);

    size = base58_decode(extendedkey, sizeof(extendedkey), extendedpubbase58);
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);
    CU_ASSERT_NSTRING_EQUAL(extendedkey, _extendedkey, EXTENDEDKEY_BYTES - 4);
}

static void test_derive_publiconly(void)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES], _extendedkey[EXTENDEDKEY_BYTES];
    ssize_t size;
    HDKey _root, *root;
    HDKey _prederivekey, *prederivekey;
    HDKey _pubhdkey, *pubhdkey;
    HDKey _derivedkey, *derivedkey;
    HDKey _derivedpub, *derivedpub;
    char prvbase58[MAX_PUBLICKEY_BASE58], pubbse58[MAX_PUBLICKEY_BASE58];
    char extendedkeybase[512];
    int i;

    root = HDKey_FromMnemonic(mnemonic, passphrase, "english", &_root);
    CU_ASSERT_PTR_NOT_NULL(root);

    prederivekey = HDKey_GetDerivedKey(root, &_prederivekey, 4, 44 | HARDENED,
            0 | HARDENED, 0 | HARDENED, 0);
    CU_ASSERT_PTR_NOT_NULL(prederivekey);

    const char *base = HDKey_SerializePrvBase58(prederivekey, extendedkeybase, sizeof(extendedkeybase));
    CU_ASSERT_PTR_NOT_NULL(base);

    pubhdkey = HDKey_FromExtendedKeyBase58(extendedkeybase, strlen(extendedkeybase) + 1, &_pubhdkey);
    CU_ASSERT_PTR_NOT_NULL(pubhdkey);

    for (i = 0; i < 100; i++) {
        derivedkey = HDKey_GetDerivedKey(prederivekey, &_derivedkey, 1, i);
        derivedpub = HDKey_GetDerivedKey(pubhdkey, &_derivedpub, 1, i);

        CU_ASSERT_STRING_EQUAL(HDKey_GetPublicKeyBase58(derivedkey, prvbase58, sizeof(prvbase58)),
                HDKey_GetPublicKeyBase58(derivedpub, pubbse58, sizeof(pubbse58)));
        CU_ASSERT_STRING_EQUAL(HDKey_GetAddress(derivedkey),
                HDKey_GetAddress(derivedpub));
    }
}

static void test_padding_privatekey(void)
{
    HDKey _root, *root;
    HDKey _identity, *identity;
    HDKey _rk, *rk;
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    char base1[256], base2[256];

    const char *expectedIDString = "iY4Ghz9tCuWvB5rNwvn4ngWvthZMNzEA7U";
    const char *newmnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";

    root = HDKey_FromMnemonic(newmnemonic, "", "english", &_root);
    CU_ASSERT_PTR_NOT_NULL(root);

    identity = HDKey_GetDerivedKey(root, &_identity, 5, 44 | HARDENED,
            0 | HARDENED, 0 | HARDENED, 0, 0);
    CU_ASSERT_PTR_NOT_NULL(identity);
    CU_ASSERT_STRING_EQUAL(expectedIDString, HDKey_GetAddress(identity));

    ssize_t size = HDKey_PaddingToExtendedPrivateKey(HDKey_GetPrivateKey(identity), PRIVATEKEY_BYTES,
        extendedkey, sizeof(extendedkey));
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);

    rk = HDKey_Deserialize(&_rk, extendedkey, size);
    CU_ASSERT_PTR_NOT_NULL(rk);

    CU_ASSERT_NSTRING_EQUAL(HDKey_GetPrivateKey(identity), HDKey_GetPrivateKey(rk), PRIVATEKEY_BYTES);
    CU_ASSERT_STRING_EQUAL(HDKey_GetPublicKeyBase58(identity, base1, sizeof(base1)),
           HDKey_GetPublicKeyBase58(identity, base2, sizeof(base2)));
}

static void test_padding_privatekey2(void)
{
    HDKey _root, *root;
    HDKey _identity, *identity;
    HDKey _rk, *rk;
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    char base1[256], base2[256];

    const char *expectedIDString = "iW3HU8fTmwkENeVT9UCEvvg3ddUD5oCxYA";
    const char *newmnemonic = "service illegal blossom voice three eagle grace agent service average knock round";

    root = HDKey_FromMnemonic(newmnemonic, "", "english", &_root);
    CU_ASSERT_PTR_NOT_NULL(root);

    identity = HDKey_GetDerivedKey(root, &_identity, 5, 44 | HARDENED,
            0 | HARDENED, 0 | HARDENED, 0, 0);
    CU_ASSERT_PTR_NOT_NULL(identity);
    CU_ASSERT_STRING_EQUAL(expectedIDString, HDKey_GetAddress(identity));

    ssize_t size = HDKey_PaddingToExtendedPrivateKey(HDKey_GetPrivateKey(identity), PRIVATEKEY_BYTES,
        extendedkey, sizeof(extendedkey));
    CU_ASSERT_EQUAL(size, EXTENDEDKEY_BYTES);

    rk = HDKey_Deserialize(&_rk, extendedkey, size);
    CU_ASSERT_PTR_NOT_NULL(rk);

    CU_ASSERT_NSTRING_EQUAL(HDKey_GetPrivateKey(identity), HDKey_GetPrivateKey(rk), PRIVATEKEY_BYTES);
    CU_ASSERT_STRING_EQUAL(HDKey_GetPublicKeyBase58(identity, base1, sizeof(base1)),
           HDKey_GetPublicKeyBase58(identity, base2, sizeof(base2)));
}

static void test_hdkey_sign_verify(void)
{
    const char *mnemonic;
    char sig[512];
    uint8_t extendedkey[EXTENDEDKEY_BYTES], data[124], digest[32];
    HDKey hk, *privateIdentity;
    HDKey _derivedkey, *derivedkey;
    ssize_t size;
    int rc, i;

    const char *newmnemonic = Mnemonic_Generate("english");
    CU_ASSERT_PTR_NOT_NULL(newmnemonic);

    privateIdentity = HDKey_FromMnemonic(newmnemonic, "", "english", &hk);
    Mnemonic_Free((void*)newmnemonic);
    CU_ASSERT_PTR_NOT_NULL(privateIdentity);

    for (i = 0; i < 10; i++) {
        memset(data, i, sizeof(data));
        derivedkey = HDKey_GetDerivedKey(privateIdentity, &_derivedkey, 5, 44 | HARDENED,
               0 | HARDENED, 0 | HARDENED, 0, i++);
        CU_ASSERT_PTR_NOT_NULL(derivedkey);

        size = sha256_digest(digest, 1, data, sizeof(data));
        CU_ASSERT_NOT_EQUAL(size, -1);

        CU_ASSERT_NOT_EQUAL(-1, ecdsa_sign_base64(sig, HDKey_GetPrivateKey(derivedkey), digest, size));
        uint8_t *sk = HDKey_GetPrivateKey(derivedkey);
        CU_ASSERT_NOT_EQUAL(-1, ecdsa_verify_base64(sig, HDKey_GetPublicKey(derivedkey), digest, size));
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
    { "test_padding_privatekey",           test_padding_privatekey         },
    { "test_padding_privatekey2",          test_padding_privatekey2        },
    { "test_hdkey_sign_verify",            test_hdkey_sign_verify          },
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

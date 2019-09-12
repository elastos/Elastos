#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <crystal.h>
#include <CUnit/Basic.h>
#include <limits.h>

#include "HDkey.h"

static char *uint256_to_string(const UInt256 u256, int reverse)
{
    static char string[64];
    int i;

    if(!reverse) {
        for (i = 0; i < sizeof(u256.u8); i++) {
            int index = i + i;
            string[index] = (char) _hexc(u256.u8[i] >> 4);
            string[index + 1] = (char) _hexc(u256.u8[i]);
        }
    }
    else {
        for (i = sizeof(u256.u8) - 1; i >= 0; i--) {
            int index = i + i -1;
            string[index] = (char) _hexc(u256.u8[i] >> 4);
            string[index - 1] = (char) _hexc(u256.u8[i]);
        }
    }

    return string;
}

static void test_privatekey(void)
{
    char *mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";
    void *seed;
    int seedLen = HDkey_GetSeedFromMnemonic(&seed, mnemonic, "", "English");
    //char *privatekey = HDkey_GetSubPrivateKey(seed, seedLen, 0, 0, 0);
    unit8_t *bin_privatekey = HDkey_GetSubPrivateKey(seed, seedLen, 0, 0, 0);

    MasterPublicKey* masterpk = HDkey_GetMasterPublicKey(seed, seedLen, 0);
    char *publickey = HDkey_GetSubPublicKey(masterpk, 0, 0);
    char *idstring = HDkey_CreateIdString(publickey);
    printf("\nget privatekey [%s]\nget publickey [%s]\nget idstring [%s]\n", privatekey, publickey, idstring);
    free(idstring);
    free(publickey);
}

static int privatekey_test_suite_init(void)
{
    return 0;
}

static int privatekey_test_suite_cleanup(void)
{
    return 0;
}

static CU_TestInfo cases[] = {
    {   "test_privatekey",              test_privatekey      },
    {   NULL,                           NULL                 }
};

static CU_SuiteInfo suite[] = {
    {   "didstore privatekey test",    didstore_storepk_test_suite_init,    didstore_storepk_test_suite_cleanup,      NULL, NULL, cases },
    {    NULL,                         NULL,                                NULL,                                     NULL, NULL, NULL  }
};

CU_SuiteInfo* didstore_store_pk_test_suite_info(void)
{
    return suite;
}
#ifndef __TEST_DID_ADAPTER_H__
#define __TEST_DID_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ela_did.h"

typedef const char* (GetPasswordCallback)(const char *walletDir, const char *walletId);

DIDAdapter *TestDIDAdapter_Create(const char *walletDir, const char *walletId,
        const char *network, GetPasswordCallback *callback);

void TestDIDAdapter_Destroy(DIDAdapter *adapter);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_DID_ADAPTER_H__ */
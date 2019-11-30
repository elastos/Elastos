#ifndef __TEST_DID_ADAPTER_H__
#define __TEST_DID_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ela_did.h"

typedef const char* (GetPasswordCallback)(const char *walletDir, const char *walletId);

DIDAdapter *TestAdapter_Create(const char *walletDir, const char *walletId,
        const char *network, const char *resolver, GetPasswordCallback *callback);

void TestAdapter_Destroy(DIDAdapter *adapter);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_DID_ADAPTER_H__ */
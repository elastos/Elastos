#ifndef __SPV_ADAPTER_H__
#define __SPV_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SpvDidAdapter SpvDidAdapter;

SpvDidAdapter *SpvDidAdapter_Create(const char *walletDir, const char *walletId,
        const char *network, const char *resolver);

void SpvDidAdapter_Destroy(SpvDidAdapter *adapter);

int SpvDidAdapter_IsAvailable(SpvDidAdapter *adapter);

int SpvDidAdapter_CreateIdTransaction(SpvDidAdapter *adapter,
        const char *payload, const char *memo, const char *password);

const char *SpvDidAdapter_Resolve(SpvDidAdapter *adapter, const char *did);

void SpvDidAdapter_FreeMemory(SpvDidAdapter *adapter, void *mem);

#ifdef __cplusplus
}
#endif

#endif /* __SPV_ADAPTER_H__ */
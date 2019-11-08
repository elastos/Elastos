#ifndef __SPV_ADAPTOR_H__
#define __SPV_ADAPTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SpvDidAdaptor SpvDidAdaptor;

SpvDidAdaptor *SpvDidAdaptor_Create(const char *walletDir, const char *walletId, const char *network);

void SpvDidAdaptor_Destroy(SpvDidAdaptor *adaptor);

int SpvDidAdaptor_CreateIdTransaction(SpvDidAdaptor *adaptor, const char *payload, const char *memo, const char *password);

const char *SpvDidAdaptor_Resolve(SpvDidAdaptor *adaptor, const char *did);

void SpvDidAdaptor_FreeMemory(SpvDidAdaptor *adaptor, void *mem);

#ifdef __cplusplus
}
#endif

#endif /* __SPV_ADAPTOR_H__ */
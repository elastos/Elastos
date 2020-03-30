#ifndef __SPV_ADAPTER_H__
#define __SPV_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(DID_STATIC)
    #define DID_API
#elif defined(DID_DYNAMIC)
    #ifdef DID_BUILD
        #if defined(_WIN32) || defined(_WIN64)
            #define DID_API         __declspec(dllexport)
        #else
            #define DID_API         __attribute__((visibility("default")))
        #endif
    #else
        #if defined(_WIN32) || defined(_WIN64)
            #define DID_API         __declspec(dllimport)
        #else
            #define DID_API         __attribute__((visibility("default")))
        #endif
    #endif
#else
    #define DID_API
#endif

typedef struct SpvDidAdapter SpvDidAdapter;

typedef void SpvTransactionCallback(const char *txid, int status,
        const char *msg, void *context);

DID_API SpvDidAdapter *SpvDidAdapter_Create(const char *walletDir,
        const char *walletId, const char *network);

DID_API void SpvDidAdapter_Destroy(SpvDidAdapter *adapter);

DID_API int SpvDidAdapter_IsAvailable(SpvDidAdapter *adapter);

DID_API const char *SpvDidAdapter_CreateIdTransaction(SpvDidAdapter *adapter,
        const char *payload, const char *memo, const char *password);

DID_API void SpvDidAdapter_CreateIdTransactionEx(SpvDidAdapter *adapter,
        const char *payload, const char *memo, int confirms,
        SpvTransactionCallback *txCallback, void *context,
        const char *password);

DID_API void SpvDidAdapter_FreeMemory(SpvDidAdapter *adapter, void *mem);

#ifdef __cplusplus
}
#endif

#endif /* __SPV_ADAPTER_H__ */

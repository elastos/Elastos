#ifndef __DUMMY_ADAPTER_H__
#define __DUMMY_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ela_did.h"

typedef struct DummyAdapter {
	DIDAdapter adapter;
	DIDResolver resolver;
	void (*reset)(struct DummyAdapter *adapter);
} DummyAdapter;

DID_API DummyAdapter *DummyAdapter_Create(void);

DID_API void DummyAdapter_Destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* __DUMMY_ADAPTER_H__ */
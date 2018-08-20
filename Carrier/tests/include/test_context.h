#ifndef __TEST_CONTEXT_H__
#define __TEST_CONTEXT_H__

#include "ela_carrier.h"
#include "ela_session.h"

#include "cond.h"

typedef struct Condition Condition;
typedef struct CarrierContextExtra CarrierContextExtra;
typedef struct SessionContextExtra SessionContextExtra;
typedef struct StreamContextExtra  StreamContextExtra;

typedef struct StreamContext {
    ElaStreamCallbacks *cbs;
    int stream_id;
    ElaStreamState state;
    uint8_t state_bits;
    Condition *cond;

    StreamContextExtra *extra;
} StreamContext;

typedef struct SessionContext {
    ElaSessionRequestCallback *request_cb;
    int request_received;
    Condition *request_cond;

    ElaSessionRequestCompleteCallback *request_complete_cb;
    int request_complete_status;
    Condition *request_complete_cond;

    ElaSession *session;

    SessionContextExtra *extra;
} SessionContext;

typedef struct CarrierContext {
    ElaCallbacks *cbs;
    ElaCarrier *carrier;
    Condition *ready_cond;
    Condition *cond;
    pthread_t thread;
    bool robot_online;

    CarrierContextExtra *extra;
} CarrierContext;

typedef struct TestContext TestContext;

struct TestContext {
    CarrierContext *carrier;
    SessionContext *session;
    StreamContext  *stream;

    void (*context_reset)(TestContext *);
};

#endif /* __TEST_CONTEXT_H__ */

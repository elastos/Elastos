#ifndef __TEST_HELPER_H__
#define __TEST_HELPER_H__

#include "ela_carrier.h"
#include "ela_session.h"

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
    volatile bool robot_online;

    CarrierContextExtra *extra;
} CarrierContext;

typedef struct TestContext TestContext;

struct TestContext {
    CarrierContext *carrier;
    SessionContext *session;
    StreamContext  *stream;

    void (*context_reset)(TestContext *);
};

#define FREE_ANYWAY(ptr) do {   \
    if ((ptr)) {                \
        free(ptr);              \
        (ptr) = NULL;           \
    }                           \
} while(0)

int test_suite_init_ext(TestContext *ctx, bool udp_disabled);

int test_suite_init(TestContext *ctx);

int test_suite_cleanup(TestContext *ctx);

int add_friend_anyway(TestContext *ctx, const char *userid, const char *address);

int remove_friend_anyway(TestContext *ctx, const char *userid);

int robot_sinit(void);

void robot_sfree(void);

const char *stream_state_name(ElaStreamState state);

void test_stream_scheme(ElaStreamType stream_type, int stream_options,
                        TestContext *context, int (*do_work_cb)(TestContext *));

const char* connection_str(enum ElaConnectionStatus status);

#endif /* __TEST_HELPER_H__ */

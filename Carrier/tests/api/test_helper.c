#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>

#include <CUnit/Basic.h>
#include "ela_carrier.h"
#include "ela_session.h"
#include "cond.h"
#include "tests.h"
#include "test_helper.h"
#include "test_assert.h"

static void* carrier_run_entry(void *arg)
{
    ElaCarrier *w = ((CarrierContext*)arg)->carrier;
    int rc;

    rc = ela_run(w, 10);
    if (rc != 0) {
        printf("Error: start carrier loop error %d.\n", ela_get_error());
        ela_kill(w);
    }

    return NULL;
}

static void carrier_idle_cb(ElaCarrier *w, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->idle)
        cbs->idle(w, context);
}

static
void carrier_connection_status_cb(ElaCarrier *w, ElaConnectionStatus status,
                                  void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    switch (status) {
        case ElaConnectionStatus_Connected:
            test_log_info("Connected to carrier network.\n");
            break;

        case ElaConnectionStatus_Disconnected:
            test_log_info("Disconnect from carrier network.\n");
            break;

        default:
            test_log_error("Error!!! Got unknown connection status %d.\n", status);
    }

    if (cbs && cbs->connection_status)
        cbs->connection_status(w, status, context);
}

static void carrier_ready_cb(ElaCarrier *w, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    test_log_info("Carrier is ready.\n");

    if (cbs && cbs->ready)
        cbs->ready(w, context);
}

static void carrier_self_info_cb(ElaCarrier *w, const ElaUserInfo *info,
                                 void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->self_info)
        cbs->self_info(w, info, context);
}

static bool carrier_friend_list_cb(ElaCarrier *w, const ElaFriendInfo* info,
                                   void* context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->friend_list)
        return cbs->friend_list(w, info, context);

    return true;
}

static void carrier_friend_info_cb(ElaCarrier *w, const char *friendid,
                                   const ElaFriendInfo *info, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->friend_info)
        cbs->friend_info(w, friendid, info, context);
}

static void carrier_friend_connection_cb(ElaCarrier *w, const char *friendid,
                                ElaConnectionStatus status, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext *)context)->cbs;

    if (cbs && cbs->friend_connection)
        cbs->friend_connection(w, friendid, status, context);
}

static void carrier_friend_presence_cb(ElaCarrier *w, const char *friendid,
                                ElaPresenceStatus presence, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->friend_presence)
        cbs->friend_presence(w, friendid, presence, context);
}

static void carrier_friend_request_cb(ElaCarrier *w, const char *userid,
                                      const ElaUserInfo *info,
                                      const char *hello, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->friend_request)
        cbs->friend_request(w, userid, info, hello, context);
}

static void carrier_friend_added_cb(ElaCarrier *w, const ElaFriendInfo *info,
                                    void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->friend_added)
        cbs->friend_added(w, info, context);
}

static void carrier_friend_removed_cb(ElaCarrier *w, const char *friendid,
                                      void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->friend_removed)
        cbs->friend_removed(w, friendid, context);
}

static void carrier_friend_message_cb(ElaCarrier *w, const char *from,
                                      const char *msg, size_t len, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->friend_message)
        cbs->friend_message(w, from, msg, len, context);
}

static void carrier_friend_invite_cb(ElaCarrier *w, const char *from,
                                     const char *data, size_t len,
                                     void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->friend_invite)
        cbs->friend_invite(w, from, data, len, context);
}

static ElaCallbacks callbacks = {
    .idle            = carrier_idle_cb,
    .connection_status = carrier_connection_status_cb,
    .ready           = carrier_ready_cb,
    .self_info       = carrier_self_info_cb,
    .friend_list     = carrier_friend_list_cb,
    .friend_info     = carrier_friend_info_cb,
    .friend_connection = carrier_friend_connection_cb,
    .friend_presence = carrier_friend_presence_cb,
    .friend_request  = carrier_friend_request_cb,
    .friend_added    = carrier_friend_added_cb,
    .friend_removed  = carrier_friend_removed_cb,
    .friend_message  = carrier_friend_message_cb,
    .friend_invite   = carrier_friend_invite_cb
};

int test_suite_init_ext(TestContext *context, bool udp_disabled)
{
    CarrierContext *wctxt = context->carrier;
    ElaOptions opts = {
        .udp_enabled = !udp_disabled,
        .persistent_location = global_config.tests.data_location,
        .bootstraps_size = global_config.bootstraps_size,
        .bootstraps = NULL
    };
    int i = 0;

    opts.bootstraps = (BootstrapNode *)calloc(1, sizeof(BootstrapNode) * opts.bootstraps_size);
    if (!opts.bootstraps) {
        test_log_error("Error: out of memory.");
        return -1;
    }

    for (i = 0 ; i < opts.bootstraps_size; i++) {
        BootstrapNode *b = &opts.bootstraps[i];
        BootstrapNode *node = global_config.bootstraps[i];

        b->ipv4 = node->ipv4;
        b->ipv6 = node->ipv6;
        b->port = node->port;
        b->public_key = node->public_key;
    }

    wctxt->carrier = ela_new(&opts, &callbacks, wctxt);
    free(opts.bootstraps);

    if (!wctxt->carrier) {
        test_log_error("Error: carrier new error (0x%x)\n", ela_get_error());
        return -1;
    }

    cond_reset(wctxt->cond);
    cond_reset(wctxt->ready_cond);

    pthread_create(&wctxt->thread, 0, &carrier_run_entry, wctxt);
    cond_wait(wctxt->ready_cond);

    return 0;
}

int test_suite_init(TestContext *context)
{
	return test_suite_init_ext(context, false);
}

int test_suite_cleanup(TestContext *context)
{
    CarrierContext *wctxt = context->carrier;

    ela_kill(wctxt->carrier);
    pthread_join(wctxt->thread, 0);
    cond_deinit(wctxt->cond);

    return 0;
}

int add_friend_anyway(TestContext *context, const char *userid,
                      const char *address)
{
    CarrierContext *wctxt = context->carrier;
    int rc;

    if (ela_is_friend(wctxt->carrier, userid)) {
        while(!wctxt->robot_online)
            usleep(500);
        return 0;
    }

    rc = ela_add_friend(wctxt->carrier, address, "auto-reply");
    if (rc < 0) {
        test_log_error("Error: attempt to add friend error.\n");
        return rc;
    }

    // wait for friend_added callback invoked.
    cond_wait(wctxt->cond);

    // wait for friend_connection (online) callback invoked.
    cond_wait(wctxt->cond);

    return 0;
}

int remove_friend_anyway(TestContext *context, const char *userid)
{
    CarrierContext *wctxt = context->carrier;
    int rc;

    if (!ela_is_friend(wctxt->carrier, userid)) {
        while (wctxt->robot_online)
            usleep(500);
        return 0;
    } else {
        while (!wctxt->robot_online)
            usleep(500);
    }

    rc = ela_remove_friend(wctxt->carrier, userid);
    if (rc < 0) {
        test_log_error("Error: remove friend error (%x)\n", ela_get_error());
        return rc;
    }

    // wait for friend_connection (online -> offline) callback invoked.
    cond_wait(wctxt->cond);

    // wait for friend_removed callback invoked.
    cond_wait(wctxt->cond);

    return 0;
}

int robot_sinit(void)
{
    return robot_ctrl("sinit\n");
}

void robot_sfree(void)
{
    robot_ctrl("sfree\n");
}

const char *stream_state_name(ElaStreamState state)
{
    const char *state_name[] = {
        "raw",
        "initialized",
        "transport ready",
        "connecting",
        "connected",
        "deactivated",
        "closed",
        "failed"
    };

    return state_name[state];
}

void test_stream_scheme(ElaStreamType stream_type, int stream_options,
                        TestContext *context, int (*do_work_cb)(TestContext *))
{
    CarrierContext *wctxt = context->carrier;
    SessionContext *sctxt = context->session;
    StreamContext *stream_ctxt = context->stream;

    int rc;
    char cmd[32];
    char result[32];

    context->context_reset(context);

    rc = add_friend_anyway(context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    rc = ela_session_init(wctxt->carrier, NULL, NULL);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    rc = robot_sinit();
    TEST_ASSERT_TRUE(rc > 0);

    rc = wait_robot_ack("%32s %32s", cmd, result);
    TEST_ASSERT_TRUE(rc == 2);
    TEST_ASSERT_TRUE(strcmp(cmd, "sinit") == 0);
    TEST_ASSERT_TRUE(strcmp(result, "success") == 0);

    sctxt->session = ela_session_new(wctxt->carrier, robotid);
    TEST_ASSERT_TRUE(sctxt->session != NULL);

    stream_ctxt->stream_id = ela_session_add_stream(sctxt->session,
                                        stream_type, stream_options,
                                        stream_ctxt->cbs, stream_ctxt);
    TEST_ASSERT_TRUE(stream_ctxt->stream_id > 0);

    cond_wait(stream_ctxt->cond);
    TEST_ASSERT_TRUE(stream_ctxt->state == ElaStreamState_initialized);
    TEST_ASSERT_TRUE(stream_ctxt->state_bits & (1 << ElaStreamState_initialized));

    rc = ela_session_request(sctxt->session, sctxt->request_complete_cb, sctxt);
    TEST_ASSERT_TRUE(rc == 0);

    cond_wait(stream_ctxt->cond);
    TEST_ASSERT_TRUE(stream_ctxt->state == ElaStreamState_transport_ready);
    TEST_ASSERT_TRUE(stream_ctxt->state_bits & (1 << ElaStreamState_transport_ready));

    rc = wait_robot_ack("%32s %32s", cmd, result);
    TEST_ASSERT_TRUE(rc == 2);
    TEST_ASSERT_TRUE(strcmp(cmd, "srequest") == 0);
    TEST_ASSERT_TRUE(strcmp(result, "received") == 0);

    rc = robot_ctrl("sreply confirm %d %d\n", stream_type, stream_options);
    TEST_ASSERT_TRUE(rc > 0);

    cond_wait(sctxt->request_complete_cond);
    TEST_ASSERT_TRUE(sctxt->request_received == 0);
    TEST_ASSERT_TRUE(sctxt->request_complete_status == 0);

    rc = wait_robot_ack("%32s %32s", cmd, result);
    TEST_ASSERT_TRUE(rc == 2);
    TEST_ASSERT_TRUE(strcmp(cmd, "sreply") == 0);
    TEST_ASSERT_TRUE(strcmp(result, "success") == 0);

    cond_wait(stream_ctxt->cond);

    if (stream_ctxt->state != ElaStreamState_connecting &&
        stream_ctxt->state != ElaStreamState_connected) {
        // if error, consume ctrl acknowlege from robot.
        wait_robot_ack("%32s %32s", cmd, result);
    }

    // Stream 'connecting' state is a transient state.
    TEST_ASSERT_TRUE(stream_ctxt->state == ElaStreamState_connecting ||
                     stream_ctxt->state == ElaStreamState_connected);
    TEST_ASSERT_TRUE(stream_ctxt->state_bits & (1 << ElaStreamState_connecting));

    rc = wait_robot_ack("%32s %32s", cmd, result);
    TEST_ASSERT_TRUE(rc == 2);
    TEST_ASSERT_TRUE(strcmp(cmd, "sconnect") == 0);
    TEST_ASSERT_TRUE(strcmp(result, "success") == 0);

    cond_wait(stream_ctxt->cond);
    TEST_ASSERT_TRUE(stream_ctxt->state == ElaStreamState_connected);
    TEST_ASSERT_TRUE(stream_ctxt->state_bits & (1 << ElaStreamState_connected));

    rc = do_work_cb ? do_work_cb(context) : 0;
    TEST_ASSERT_TRUE(rc == 0);

    rc = ela_session_remove_stream(sctxt->session, stream_ctxt->stream_id);
    TEST_ASSERT_TRUE(rc == 0);
    stream_ctxt->stream_id = -1;

    cond_wait(stream_ctxt->cond);
    TEST_ASSERT_TRUE(stream_ctxt->state == ElaStreamState_closed);
    TEST_ASSERT_TRUE(stream_ctxt->state_bits & (1 << ElaStreamState_closed));

cleanup:
    if (stream_ctxt->stream_id > 0) {
        ela_session_remove_stream(sctxt->session, stream_ctxt->stream_id);
        stream_ctxt->stream_id = -1;
    }

    if (sctxt->session) {
        ela_session_close(sctxt->session);
        sctxt->session = NULL;
    }

    ela_session_cleanup(wctxt->carrier);
    robot_sfree();
}

const char* connection_str(enum ElaConnectionStatus status)
{
    const char* str = NULL;

    switch (status) {
        case ElaConnectionStatus_Connected:
            str = "connected";
            break;
        case ElaConnectionStatus_Disconnected:
            str = "disconnected";
            break;
        default:
            str = "unknown";
            break;
    }
    return str;
}

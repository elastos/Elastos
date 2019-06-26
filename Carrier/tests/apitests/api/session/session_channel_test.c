/*
 * Copyright (c) 2018 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <time.h>
#include <pthread.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <CUnit/Basic.h>
#include <crystal.h>

#include "ela_carrier.h"
#include "ela_session.h"

#include "cond.h"
#include "test_helper.h"
#include "test_assert.h"

#define MAX_CHANNEL_COUNT 128

static inline void wakeup(void* context)
{
    cond_signal(((CarrierContext *)context)->cond);
}

static void ready_cb(ElaCarrier *w, void *context)
{
    cond_signal(((CarrierContext *)context)->ready_cond);
}

static
void friend_added_cb(ElaCarrier *w, const ElaFriendInfo *info, void *context)
{
    wakeup(context);
}

static void friend_removed_cb(ElaCarrier *w, const char *friendid, void *context)
{
    wakeup(context);
}

static void friend_connection_cb(ElaCarrier *w, const char *friendid,
                                 ElaConnectionStatus status, void *context)
{
    CarrierContext *wctxt = (CarrierContext *)context;

    status_cond_signal(wctxt->friend_status_cond, status);

    vlogD("Robot connection status changed -> %s", connection_str(status));
}

static ElaCallbacks callbacks = {
    .idle            = NULL,
    .connection_status = NULL,
    .ready           = ready_cb,
    .self_info       = NULL,
    .friend_list     = NULL,
    .friend_connection = friend_connection_cb,
    .friend_info     = NULL,
    .friend_presence = NULL,
    .friend_request  = NULL,
    .friend_added    = friend_added_cb,
    .friend_removed  = friend_removed_cb,
    .friend_message  = NULL,
    .friend_invite   = NULL
};

static Condition DEFINE_COND(carrier_ready_cond);
static Condition DEFINE_COND(carrier_cond);
static StatusCondition DEFINE_STATUS_COND(friend_status_cond);

static CarrierContext carrier_context = {
    .cbs = &callbacks,
    .carrier = NULL,
    .ready_cond = &carrier_ready_cond,
    .cond = &carrier_cond,
    .friend_status_cond = &friend_status_cond,
    .extra = NULL
};

static void session_request_complete_callback(ElaSession *ws, const char *bundle, int status,
                const char *reason, const char *sdp, size_t len, void *context)
{
    SessionContext *sctxt = (SessionContext *)context;

    vlogD("Session complete, status: %d, reason: %s", status,
          reason ? reason : "null");

    sctxt->request_complete_status = status;

    if (status == 0) {
        int rc;

        rc = ela_session_start(ws, sdp, len);
        CU_ASSERT_EQUAL(rc, 0);
    }

    cond_signal(sctxt->request_complete_cond);
}

static Condition DEFINE_COND(session_request_complete_cond);

static SessionContext session_context = {
    .request_cb = NULL,
    .request_received = 0,
    .request_cond = NULL,

    .request_complete_cb = session_request_complete_callback,
    .request_complete_status = -1,
    .request_complete_cond = &session_request_complete_cond,

    .session = NULL,
    .extra   = NULL,
};

struct StreamContextExtra {
    int channel_error_state[MAX_CHANNEL_COUNT];
    int channel_id[MAX_CHANNEL_COUNT];
    int channel_count;

    Condition *channel_cond;

    // For bulk write routine.
    int max_data_sz;
    int min_data_sz;
    int max_pkt_sz;
    int min_pkt_sz;

    int return_val;
};

static Condition DEFINE_COND(channel_cond);

static StreamContextExtra stream_extra = {
    .channel_error_state = { 0 },
    .channel_id = { 0 },
    .channel_count = 0,
    .channel_cond = &channel_cond
};

static void stream_on_data(ElaSession *ws, int stream,
                           const void *data, size_t len, void *context)
{
    vlogD("Stream [%d] received data [%.*s]", stream, (int)len, (char*)data);
}

static void stream_state_changed(ElaSession *ws, int stream,
                                 ElaStreamState state, void *context)
{
    StreamContext *stream_ctxt = (StreamContext *)context;

    stream_ctxt->state = state;
    stream_ctxt->state_bits |= (1 << state);

    vlogD("Stream [%d] state changed to: %s", stream, stream_state_name(state));

    cond_signal(stream_ctxt->cond);
}

static bool on_channel_open(ElaSession *ws, int stream, int channel,
                            const char *cookie, void *context)
{
    vlogD("Stream request open new channel %d.", channel);
    return true;
}

static
void on_channel_opened(ElaSession *ws, int stream, int channel, void *context)
{
    StreamContextExtra *extra = ((StreamContext *)context)->extra;

    vlogD("Channel %d opened.", channel);

    //TODO:
    extra->channel_error_state[channel - 1] = 0;
    extra->channel_id[channel - 1] = channel;
    extra->channel_count++;

    cond_signal(extra->channel_cond);
}

static void on_channel_close(ElaSession *ws, int stream, int channel,
                             CloseReason reason, void *context)
{
    StreamContextExtra *extra = ((StreamContext *)context)->extra;

    const char *state_name[] = {
        "Normal",
        "Timeout",
        "Error"
    };

    vlogD("Channel %d closeing with %s.", channel, state_name[reason]);

    // TODO:
    if (reason == CloseReason_Error || reason == CloseReason_Timeout)
        extra->channel_error_state[channel - 1] = 1;

    cond_signal(extra->channel_cond);
}

static bool on_channel_data(ElaSession *ws, int stream, int channel,
                            const void *data, size_t len, void *context)
{
    StreamContextExtra *extra = ((StreamContext *)context)->extra;

    vlogD("stream [%d] channel [%d] received data [%.*s]",
          stream, channel, (int)len, (char*)data);

    cond_signal(extra->channel_cond);
    return true;
}

static void on_channel_pending(ElaSession *ws, int stream, int channel,
                               void *context)
{
    StreamContextExtra *extra = ((StreamContext *)context)->extra;

    vlogD("stream [%d] channel [%d] pend data.", stream, channel);

    cond_signal(extra->channel_cond);
}

static void on_channel_resume(ElaSession *ws, int stream, int channel,
                              void *context)
{
    StreamContextExtra *extra = ((StreamContext *)context)->extra;

    vlogD("stream [%d] channel [%d] resume data.", stream, channel);

    cond_signal(extra->channel_cond);
}

static ElaStreamCallbacks stream_callbacks = {
    .stream_data = stream_on_data,
    .state_changed = stream_state_changed,
    .channel_open = on_channel_open,
    .channel_opened = on_channel_opened,
    .channel_close = on_channel_close,
    .channel_data = on_channel_data,
    .channel_pending = on_channel_pending,
    .channel_resume = on_channel_resume
};

static Condition DEFINE_COND(stream_cond);

static StreamContext stream_context = {
    .cbs = &stream_callbacks,
    .stream_id = -1,
    .state = 0,
    .state_bits = 0,
    .cond = &stream_cond,
    .extra = &stream_extra
};

static void test_context_reset(TestContext *context)
{
    SessionContext *session = context->session;
    StreamContext *stream = context->stream;

    cond_reset(context->carrier->cond);
    status_cond_reset(context->carrier->friend_status_cond);

    cond_reset(session->request_complete_cond);

    session->request_received = 0;
    session->request_complete_status = -1;
    session->session = NULL;

    cond_reset(context->stream->cond);

    stream->stream_id = -1;
    stream->state = 0;
    stream->state_bits = 0;

    cond_reset(context->stream->extra->channel_cond);
}

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = &session_context,
    .stream  = &stream_context,

    .context_reset = test_context_reset
};

static int write_channel_data(ElaSession *session, int stream, int channel,
                              char *data, size_t len)
{
    size_t left;
    char *pos;
    int rc;

    left = len;
    pos  = data;

    while(left > 0) {
        rc = (int)ela_stream_write_channel(session, stream, channel, pos, left);
        if (rc == 0) {
            assert(0);
        } else if (rc < 0) {
            if (ela_get_error() == ELA_GENERAL_ERROR(ELAERR_BUSY)) {
                usleep(100);
                continue;
            } else {
                vlogE("Write channel data failed (0x%x)",
                      ela_get_error());
                return -1;
            }
        } else {
            pos += rc;
            left -= rc;
        }
    }

    return 0;
}

static void *bulk_channel_write_routine(void *arg)
{
    TestContext *ctxt = (TestContext *)arg;
    StreamContextExtra *extra = ctxt->stream->extra;
    int data_sz;
    int pkt_sz;
    int pkt_count;
    char *packet;
    int i;
    struct timeval start;
    struct timeval end;
    int duration;
    float speed;

    data_sz = rand() % (extra->max_data_sz - extra->min_data_sz) + extra->min_data_sz;
    pkt_sz  = rand() % (extra->max_pkt_sz - extra->min_pkt_sz) + extra->min_pkt_sz;
    pkt_count = data_sz/pkt_sz + 1;

    packet = (char *)alloca(pkt_sz);
    memset(packet, 'D', pkt_sz);

    vlogD("Begin sending data...");
    vlogD("Stream %d channel %d send %d packets in total and %d bytes "
          "per packet.", ctxt->stream->stream_id,
          extra->channel_id[0], pkt_count, pkt_sz);

    gettimeofday(&start, NULL);
    for (i = 0; i < pkt_count; i++) {
        int rc;

        rc = write_channel_data(ctxt->session->session, ctxt->stream->stream_id,
                                extra->channel_id[0], packet, pkt_sz);
        if (rc < 0)
            return NULL;

        if (i % 1000 == 0)
            vlogD(".");
    }
    gettimeofday(&end, NULL);

    duration = (int)((end.tv_sec - start.tv_sec) * 1000000 +
                     (end.tv_usec - start.tv_usec)) / 1000 + 1;
    speed = (((float)(pkt_sz * pkt_count) / duration) * 1000) / 1024;

    vlogD("Finish! Total %"PRIu64" bytes in %d.%d seconds. %.2f KB/s",
          (uint64_t)(pkt_sz * pkt_count),
          (int)(duration / 1000), (int)(duration % 1000), speed);

    extra->return_val = 0;
    return NULL;
}

static int do_bulk_channel_write(TestContext *context)
{
    StreamContextExtra *extra = context->stream->extra;
    pthread_t thread;
    int rc;

    extra->max_data_sz = 1024*1024*100;
    extra->min_data_sz = 1024*1024;
    extra->max_pkt_sz  = 1900;
    extra->min_pkt_sz  = 1024;
    extra->return_val = -1;

    rc = pthread_create(&thread, NULL, bulk_channel_write_routine, context);
    if (rc != 0) {
        vlogE("create thread failed.");
        return -1;
    }

    pthread_join(thread, NULL);
    return extra->return_val;
}

static void *bulk_multiple_channels_write_routine(void *arg)
{
    TestContext *ctxt = (TestContext *)arg;
    StreamContextExtra *extra = ctxt->stream->extra;
    int data_sz;
    int pkt_sz;
    int pkt_count;
    char *packet;
    int i;
    int j;
    struct timeval start;
    struct timeval end;
    int duration;

    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
        extra->channel_count = i + 1;
        extra->channel_id[i] = ela_stream_open_channel(ctxt->session->session,
                                            ctxt->stream->stream_id, "cookie");
        if (extra->channel_id[i] < 0) {
            vlogE("Open new channel %d failed.", i + 1);
            //reclaim opened channels
            for (i = 0; i < extra->channel_count; i++)
                ela_stream_close_channel(ctxt->session->session,
                                ctxt->stream->stream_id, extra->channel_id[i]);
            extra->channel_count = 0;
            return NULL;
        } else {
            vlogD("Open channel [%d] succeeded (id:%d)", i, extra->channel_id[i]);
            cond_wait(extra->channel_cond);
        }
    }

    data_sz = rand() % (extra->max_data_sz - extra->min_data_sz) + extra->min_data_sz;
    pkt_sz  = rand() % (extra->max_pkt_sz - extra->min_pkt_sz) + extra->min_pkt_sz;
    pkt_count = data_sz/pkt_sz + 1;

    packet = (char *)alloca(pkt_sz);
    memset(packet, 'D', pkt_sz);

    vlogD("Open new 128 channels successfully.");
    vlogD("Begin to write data.....");

    gettimeofday(&start, NULL);

    for (i = 0; i < MAX_CHANNEL_COUNT / 2; i++) {
        int rc;

        for (j = 0; j < pkt_count; j++) {
            rc = write_channel_data(ctxt->session->session, ctxt->stream->stream_id,
                                    extra->channel_id[i], packet, pkt_sz);
            if (rc < 0) {
                for (i = 0; i < MAX_CHANNEL_COUNT; i++)
                    ela_stream_close_channel(ctxt->session->session,
                                    ctxt->stream->stream_id, extra->channel_id[i]);
                return NULL;
            }
        }
    }

    for (j = 0; j < pkt_count; j++) {
        for (i = MAX_CHANNEL_COUNT / 2; i < MAX_CHANNEL_COUNT; i++) {
            int rc;

            rc = write_channel_data(ctxt->session->session, ctxt->stream->stream_id,
                                    extra->channel_id[i], packet, pkt_sz);
            if (rc < 0) {
                for (i = 0; i < MAX_CHANNEL_COUNT; i++)
                    ela_stream_close_channel(ctxt->session->session,
                                    ctxt->stream->stream_id, extra->channel_id[i]);
                return NULL;
            }
        }
    }

    gettimeofday(&end, NULL);

    duration = (int)((end.tv_sec - start.tv_sec) * 1000000 +
                     (end.tv_usec - start.tv_usec)) / 1000 + 1;

    vlogD("Finish! Total 128 channel write data bytes in %d.%d seconds.",
          (int)(duration / 1000), (int)(duration % 1000));

    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
        ela_stream_close_channel(ctxt->session->session, ctxt->stream->stream_id,
                                     extra->channel_id[i]);
    }

    extra->channel_count = 0;
    extra->return_val = 0;
    return NULL;
}

static int do_bulk_multiple_channels_write(TestContext *context)
{
    StreamContextExtra *extra = context->stream->extra;
    pthread_t thread;

    extra->max_data_sz = 1024*1024;
    extra->min_data_sz = 1024*128;
    extra->max_pkt_sz  = 1900;
    extra->min_pkt_sz  = 1024;
    extra->return_val = -1;

    int rc = pthread_create(&thread, NULL, bulk_multiple_channels_write_routine,
                            context);
    if (rc != 0) {
        vlogE("create thread failed.");
        return -1;
    }

    pthread_join(thread, NULL);
    return extra->return_val;
}

static int bulk_write_channel_internal(TestContext *context)
{
    StreamContextExtra *extra = context->stream->extra;
    int rc;

    char cmd[32];
    char result[32];

    /*send command 'copen'*/
    rc = write_cmd("cready2open confirm\n");
    TEST_ASSERT_TRUE(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    TEST_ASSERT_TRUE(rc == 2);
    TEST_ASSERT_TRUE(strcmp(cmd, "cready2open") == 0);
    TEST_ASSERT_TRUE(strcmp(result, "success") == 0);

    /*open channel*/
    extra->channel_id[0] = ela_stream_open_channel(context->session->session,
                                            context->stream->stream_id, "cookie");
    TEST_ASSERT_TRUE(extra->channel_id[0] > 0);

    cond_wait(extra->channel_cond);
    TEST_ASSERT_TRUE(extra->channel_error_state[0] == 0);

    /*pend*/
    rc = write_cmd("cpend\n");
    TEST_ASSERT_TRUE(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    TEST_ASSERT_TRUE(rc == 2);
    TEST_ASSERT_TRUE(strcmp(cmd, "cpend") == 0);
    TEST_ASSERT_TRUE(strcmp(result, "success") == 0);

    cond_wait(extra->channel_cond);

    rc = write_cmd("cresume\n");
    TEST_ASSERT_TRUE(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    TEST_ASSERT_TRUE(rc == 2);
    TEST_ASSERT_TRUE(strcmp(cmd, "cresume") == 0);
    TEST_ASSERT_TRUE(strcmp(result, "success") == 0);

    cond_wait(extra->channel_cond);

    rc =  do_bulk_channel_write(context);
    TEST_ASSERT_TRUE(rc == 0);

    if (rc < 0)
        vlogE("stream channel write failed.");

    rc = ela_stream_close_channel(context->session->session,
                                  context->stream->stream_id, extra->channel_id[0]);
    extra->channel_id[0] = -1;
    TEST_ASSERT_TRUE(rc == 0);

    cond_wait(extra->channel_cond);
    TEST_ASSERT_TRUE(extra->channel_error_state[0] == 0);

    return 0;

cleanup:
    if (extra->channel_id[0]> 0)
        ela_stream_close_channel(context->session->session,
                            context->stream->stream_id, extra->channel_id[0]);
    return -1;
}

static int bulk_write_multiple_channels_internal(TestContext *context)
{
    int rc;
    char cmd[32];
    char result[32];

    /*send command 'copen'*/
    rc = write_cmd("cready2open confirm\n");

    rc = read_ack("%32s %32s", cmd, result);
    TEST_ASSERT_TRUE(rc > 0);
    TEST_ASSERT_TRUE(strcmp(cmd, "cready2open") == 0);
    TEST_ASSERT_TRUE(strcmp(result, "success") == 0);

    rc = do_bulk_multiple_channels_write(context);
    TEST_ASSERT_TRUE(rc == 0);
    return 0;

cleanup:
    return -1;
}

static inline
void channel_bulk_write(int stream_options)
{
    test_stream_scheme(ElaStreamType_text, stream_options,
                       &test_context, bulk_write_channel_internal);
}

static inline
void multiple_channels_bulk_write(int stream_options)
{
    test_stream_scheme(ElaStreamType_text, stream_options,
                       &test_context, bulk_write_multiple_channels_internal);
}

static void test_session_channel(void)
{
    int stream_options = 0;
    stream_options |= ELA_STREAM_MULTIPLEXING;

    channel_bulk_write(stream_options);
}

static void test_session_channel_plain(void)
{
    int stream_options = 0;
    stream_options |= ELA_STREAM_MULTIPLEXING;
    stream_options |= ELA_STREAM_PLAIN;

    channel_bulk_write(stream_options);
}

static void test_session_channel_reliable(void)
{
    int stream_options = 0;
    stream_options |= ELA_STREAM_MULTIPLEXING;
    stream_options |= ELA_STREAM_RELIABLE;

    channel_bulk_write(stream_options);
}

static void test_session_channel_reliable_plain(void)
{
    int stream_options = 0;
    stream_options |= ELA_STREAM_MULTIPLEXING;
    stream_options |= ELA_STREAM_PLAIN;
    stream_options |= ELA_STREAM_RELIABLE;

    channel_bulk_write(stream_options);
}

static void test_session_multiple_channels(void)
{
    int stream_options = 0;
    stream_options |= ELA_STREAM_MULTIPLEXING;

    multiple_channels_bulk_write(stream_options);
}

static void test_session_multiple_channels_plain(void)
{
    int stream_options = 0;
    stream_options |= ELA_STREAM_MULTIPLEXING;
    stream_options |= ELA_STREAM_PLAIN;

    multiple_channels_bulk_write(stream_options);
}

static void test_session_multiple_channels_reliable(void)
{
    int stream_options = 0;
    stream_options |= ELA_STREAM_MULTIPLEXING;
    stream_options |= ELA_STREAM_RELIABLE;

    multiple_channels_bulk_write(stream_options);
}

static void test_session_multiple_channels_reliable_plain(void)
{
    int stream_options = 0;
    stream_options |= ELA_STREAM_MULTIPLEXING;
    stream_options |= ELA_STREAM_PLAIN;
    stream_options |= ELA_STREAM_RELIABLE;

    multiple_channels_bulk_write(stream_options);
}

static CU_TestInfo cases[] = {
    { "test_session_channel", test_session_channel },
    { "test_session_channel_plain", test_session_channel_plain },
    { "test_session_channel_reliable", test_session_channel_reliable },
    { "test_session_channel_plain_reliable", test_session_channel_reliable_plain },

    { "test_session_multiple_channels", test_session_multiple_channels },
    { "test_session_multiple_channels_plain", test_session_multiple_channels_plain },
    { "test_session_multiple_channels_reliable", test_session_multiple_channels_reliable },
    { "test_session_multiple_channels_reliable_plain", test_session_multiple_channels_reliable_plain },

    { NULL, NULL }
};

CU_TestInfo *session_channel_test_get_cases(void)
{
    return cases;
}

int session_channel_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0)
        CU_FAIL("Error: test suite initialize error");

    srand((unsigned int)time(NULL));

    return rc;
}

int session_channel_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

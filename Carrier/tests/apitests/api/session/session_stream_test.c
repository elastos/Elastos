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

static struct CarrierContext carrier_context = {
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

static struct SessionContext session_context = {
    .request_cb = NULL,
    .request_received = 0,
    .request_cond = NULL,

    .request_complete_cb = session_request_complete_callback,
    .request_complete_status = -1,
    .request_complete_cond = &session_request_complete_cond,

    .session = NULL,
    .extra   = NULL
};

struct StreamContextExtra {
    int packet_size;
    int packet_count;
    int return_val;
};

static StreamContextExtra stream_extra = {
    .packet_size = 0,
    .packet_count = 0,
    .return_val = -1,
};

static void stream_on_data(ElaSession *ws, int stream, const void *data,
                           size_t len, void *context)
{
    vlogD("Stream [%d] received data [%.*s]", stream, (int)len, (char*)data);
}

static void stream_state_changed(ElaSession *ws, int stream,
                                 ElaStreamState state, void *context)
{
    StreamContext *sc = (StreamContext *)context;

    sc->state = state;
    sc->state_bits |= (1 << state);

    vlogD("Stream [%d] state changed to: %s", stream, stream_state_name(state));

    cond_signal(sc->cond);
}

static ElaStreamCallbacks stream_callbacks = {
    .stream_data = stream_on_data,
    .state_changed = stream_state_changed
};

static Condition DEFINE_COND(stream_cond);

static StreamContext stream_context = {
    .cbs = &stream_callbacks,
    .stream_id = -1,
    .state = 0,
    .state_bits = 0,
    .cond = &stream_cond,
    .extra = &stream_extra,
};

static void test_context_reset(TestContext *context)
{
    SessionContext *session = context->session;
    StreamContext *stream = context->stream;

    cond_reset(session->request_complete_cond);

    session->request_received = 0;
    session->request_complete_status = -1;
    session->session = NULL;

    cond_reset(context->stream->cond);

    stream->stream_id = -1;
    stream->state = 0;
    stream->state_bits = 0;
}

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = &session_context,
    .stream  = &stream_context,

    .context_reset = test_context_reset
};

static void *bulk_write_routine(void *arg)
{
    ssize_t rc;
    int i;
    SessionContext *sctxt = ((TestContext *)arg)->session;
    StreamContext *stream_ctxt = ((TestContext *)arg)->stream;
    StreamContextExtra *extra = stream_ctxt->extra;
    char *packet;
    struct timeval start, end;
    int duration;
    float speed;

    packet = (char *)alloca(extra->packet_size);
    memset(packet, 'D', extra->packet_size);

    vlogD("Begin sending data...");
    vlogD("stream %d send: total %d packets and %d bytes per packet.",
          stream_ctxt->stream_id, extra->packet_count, extra->packet_size);

    gettimeofday(&start, NULL);
    for (i = 0; i < extra->packet_count; i++) {
        size_t total = extra->packet_size;
        size_t sent = 0;

        do {
            rc = ela_stream_write(sctxt->session, stream_ctxt->stream_id,
                                  (const char*)(packet + sent),
                                  total - sent);
            if (rc < 0) { //TODO: consider condition rc == 0.
                if (ela_get_error() == ELA_GENERAL_ERROR(ELAERR_BUSY)) {
                    usleep(100);
                    continue;
                }
                else {
                    vlogE("Write data failed (0x%x)", ela_get_error());
                    return NULL;
                }
            }

            sent += rc;
        } while (sent < total);

        if (i % 1000 == 0)
            vlogD(".");
    }
    gettimeofday(&end, NULL);

    duration = (int)((end.tv_sec - start.tv_sec) * 1000000 +
                     (end.tv_usec - start.tv_usec)) / 1000 + 1;
    speed = (((float)(extra->packet_size * extra->packet_count) / duration) * 1000) / 1024;

    vlogI("Total %" PRIu64 " bytes in %d.%d seconds. %.2f KB/s",
          (uint64_t)(extra->packet_size * extra->packet_count),
          (int)(duration / 1000), (int)(duration % 1000), speed);

    extra->return_val = 0;
    return NULL;
}

static int do_bulk_write(TestContext *context)
{
#define MIN_DATA_SIZE 1024*1024*10
#define MAX_DATA_SIZE 1024*1024*100

#define MIN_PACKET_SIZE 1024
#define MAX_PACKET_SIZE 2048

    StreamContextExtra *extra = context->stream->extra;
    pthread_t thread;
    int rc;

    int send_size;
    int packet_size;
    int packet_count;

    //beginning to write message
    send_size = rand() % (MAX_DATA_SIZE - MIN_DATA_SIZE) + MIN_DATA_SIZE;
    packet_size = rand() % (MAX_PACKET_SIZE - MIN_PACKET_SIZE) + MIN_PACKET_SIZE;
    packet_count = send_size/packet_size + 1;

    extra->packet_size = packet_size;
    extra->packet_count = packet_count;
    extra->return_val = -1;

    rc = pthread_create(&thread, NULL, bulk_write_routine, context);
    if (rc != 0) {
        vlogE("create thread failed.");
        return -1;
    }

    pthread_join(thread, NULL);

    return extra->return_val;
}

static inline
void test_stream_write(int stream_options)
{
    test_stream_scheme(ElaStreamType_text, stream_options,
                       &test_context, do_bulk_write);

}

static void test_stream_unreliable(void)
{
    test_stream_write(0);
}

static void test_stream_unreliable_plain(void)
{
    test_stream_write(ELA_STREAM_PLAIN);
}

static void test_stream_reliable(void)
{
    test_stream_write(ELA_STREAM_RELIABLE);
}

static void test_stream_reliable_plain(void)
{
    int stream_options = 0;

    stream_options |= ELA_STREAM_PLAIN;
    stream_options |= ELA_STREAM_RELIABLE;

    test_stream_write(stream_options);
}

static void test_stream_multiplexing(void)
{
    int stream_options = 0;

    stream_options |= ELA_STREAM_MULTIPLEXING;

    test_stream_write(stream_options);
}

static void test_stream_plain_multiplexing(void)
{
    int stream_options = 0;

    stream_options |= ELA_STREAM_PLAIN;
    stream_options |= ELA_STREAM_MULTIPLEXING;

    test_stream_write(stream_options);
}

static void test_stream_reliable_multiplexing(void)
{
    int stream_options = 0;

    stream_options |= ELA_STREAM_RELIABLE;
    stream_options |= ELA_STREAM_MULTIPLEXING;

    test_stream_write(stream_options);
}

static void test_stream_reliable_plain_multiplexing(void)
{
    int stream_options = 0;

    stream_options |= ELA_STREAM_PLAIN;
    stream_options |= ELA_STREAM_RELIABLE;
    stream_options |= ELA_STREAM_MULTIPLEXING;

    test_stream_write(stream_options);
}

static void test_stream_reliable_portforwarding(void)
{
    int stream_options = 0;

    stream_options |= ELA_STREAM_RELIABLE;
    stream_options |= ELA_STREAM_MULTIPLEXING;
    stream_options |= ELA_STREAM_PORT_FORWARDING;

    test_stream_write(stream_options);
}

static void test_stream_reliable_plain_portforwarding(void)
{
    int stream_options = 0;

    stream_options |= ELA_STREAM_PLAIN;
    stream_options |= ELA_STREAM_RELIABLE;
    stream_options |= ELA_STREAM_MULTIPLEXING;
    stream_options |= ELA_STREAM_PORT_FORWARDING;

    test_stream_write(stream_options);
}

static CU_TestInfo cases[] = {
    { "test_stream", test_stream_unreliable },
    { "test_stream_plain", test_stream_unreliable_plain },
    { "test_stream_reliable", test_stream_reliable },
    { "test_stream_reliable_plain", test_stream_reliable_plain },
    { "test_stream_multiplexing", test_stream_multiplexing },
    { "test_stream_plain_multiplexing", test_stream_plain_multiplexing },
    { "test_stream_reliable_multiplexing", test_stream_reliable_multiplexing },
    { "test_stream_reliable_plain_multiplexing", test_stream_reliable_plain_multiplexing },
    { "test_stream_reliable_portforwarding", test_stream_reliable_portforwarding },
    { "test_stream_reliable_plain_portforwarding", test_stream_reliable_plain_portforwarding },

    { NULL, NULL }
};

CU_TestInfo *session_stream_test_get_cases(void)
{
    return cases;
}

int session_stream_test_suite_init(void)
{
    int rc;

    srand((unsigned int)time(NULL));

    rc = test_suite_init(&test_context);
    if (rc < 0)
        CU_FAIL("Error: test suite initialize error");

    return rc;
}

int session_stream_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

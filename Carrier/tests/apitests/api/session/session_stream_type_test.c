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
    .extra = NULL,
};

static void stream_on_data(ElaSession *ws, int stream, const void *data,
                           size_t len, void *context)
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

static ElaStreamCallbacks stream_callbacks = {
    .stream_data = stream_on_data,
    .state_changed = stream_state_changed
};

static Condition DEFINE_COND(stream_cond);

struct StreamContextExtra {
    int stream_type;
};

static StreamContextExtra stream_extra = {
    .stream_type = -1,
};

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
}

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = &session_context,
    .stream  = &stream_context,

    .context_reset = test_context_reset
};


static
void check_unimplemented_stream_type(ElaStreamType stream_type,
                    TestContext *context)
{
    CarrierContext *wctxt = context->carrier;
    SessionContext *sctxt = context->session;
    StreamContext *stream_ctxt = context->stream;
    int rc;

    context->context_reset(context);

    rc = add_friend_anyway(context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    rc = ela_session_init(wctxt->carrier);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    sctxt->session = ela_session_new(wctxt->carrier, robotid);
    TEST_ASSERT_TRUE(sctxt->session != NULL);

    stream_ctxt->stream_id = ela_session_add_stream(sctxt->session,
                        stream_type, 0, stream_ctxt->cbs, stream_ctxt);
    TEST_ASSERT_TRUE(stream_ctxt->stream_id < 0);
    TEST_ASSERT_TRUE(ela_get_error() == ELA_GENERAL_ERROR(ELAERR_NOT_IMPLEMENTED));

cleanup:
    if (sctxt->session) {
        ela_session_close(sctxt->session);
        sctxt->session = NULL;
    }

    ela_session_cleanup(wctxt->carrier);
}

static int check_supported_stream_type(TestContext *context)
{
    SessionContext *sctxt = context->session;
    StreamContext *stream_ctxt = context->stream;
    ElaStreamType real_type;
    int rc;

    rc = ela_stream_get_type(sctxt->session, stream_ctxt->stream_id, &real_type);
    TEST_ASSERT_TRUE(rc == 0);
    TEST_ASSERT_TRUE(stream_ctxt->extra->stream_type == (int)real_type);

    return 0;

cleanup:
    return -1;
}

static void test_unimplemented_stream_type(ElaStreamType stream_type)
{
    check_unimplemented_stream_type(stream_type, &test_context);
}

static void test_supported_stream_type(ElaStreamType stream_type)
{

    test_context.stream->extra->stream_type = stream_type;
    test_stream_scheme(stream_type, 0, &test_context,
                       check_supported_stream_type);
}

static void test_stream_audio_type(void)
{
    test_unimplemented_stream_type(ElaStreamType_audio);
}

static void test_stream_video_type(void)
{
    test_unimplemented_stream_type(ElaStreamType_video);
}

static void test_stream_text_type(void)
{
    test_supported_stream_type(ElaStreamType_text);
}

static void test_stream_application_type(void)
{
    test_supported_stream_type(ElaStreamType_application);
}

static void test_stream_message_type(void)
{
    test_supported_stream_type(ElaStreamType_message);
}

static CU_TestInfo cases[] = {
    { "test_stream_audio_type", test_stream_audio_type },
    { "test_stream_video_type", test_stream_video_type },
    { "test_stream_text_type",  test_stream_text_type  },
    { "test_stream_application_type", test_stream_application_type },
    { "test_stream_message_type", test_stream_message_type },

    { NULL, NULL }
};

CU_TestInfo *session_stream_type_test_get_cases(void)
{
    return cases;
}

int session_stream_type_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0)
        CU_FAIL("Error: test suite initialize error");

    return rc;
}

int session_stream_type_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

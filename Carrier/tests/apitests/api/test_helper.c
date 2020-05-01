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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <CUnit/Basic.h>
#include <crystal.h>

#include "ela_carrier.h"
#include "ela_session.h"

#include "config.h"
#include "cond.h"
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
            vlogI("Connected to Carrier network.");
            break;

        case ElaConnectionStatus_Disconnected:
            vlogI("Disconnect from Carrier network.");
            break;

        default:
            vlogE("Error!!! Got unknown connection status %d.", status);
    }

    if (cbs && cbs->connection_status)
        cbs->connection_status(w, status, context);
}

static void carrier_ready_cb(ElaCarrier *w, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    vlogI("Carrier is ready.");

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
    CarrierContext *wctx = (CarrierContext*)context;
    int rc;

    if (!strcmp(hello, "auto-reply")) {
        rc = ela_accept_friend(w, userid);
        if (rc < 0) {
            vlogE("Accept friend request from %s error (0x%x)",
                  userid, ela_get_error());
            status_cond_signal(wctx->friend_status_cond, ElaConnectionStatus_Disconnected);
        }
        return;
    }

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
                                      const void *msg, size_t len,
                                      int64_t timestamp, bool is_offline,
                                      void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->friend_message)
        cbs->friend_message(w, from, msg, len, timestamp, is_offline, context);
}

static void carrier_friend_invite_cb(ElaCarrier *w, const char *from,
                                     const char *bundle,
                                     const void *data, size_t len,
                                     void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->friend_invite)
        cbs->friend_invite(w, from, bundle, data, len, context);
}

static void group_invite_cb(ElaCarrier *w, const char *from,
                         const void *cookie, size_t len, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->group_invite)
        cbs->group_invite(w, from, cookie, len, context);
}

static void group_connected_cb(ElaCarrier *w, const char *groupid, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->group_callbacks.group_connected)
        cbs->group_callbacks.group_connected(w, groupid, context);
}

static void group_message_cb(ElaCarrier *w, const char *groupid,
                          const char *from, const void *message, size_t length,
                          void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->group_callbacks.group_message)
        cbs->group_callbacks.group_message(w, groupid, from, message, length, context);
}

static void peer_name_cb(ElaCarrier *w, const char *groupid,
                      const char *peerid, const char *peer_name,
                      void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->group_callbacks.peer_name)
        cbs->group_callbacks.peer_name(w, groupid, peerid, peer_name, context);
}

static void group_title_cb(ElaCarrier *w, const char *groupid,
                        const char *from, const char *title, void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->group_callbacks.group_title)
        cbs->group_callbacks.group_title(w, groupid, from, title, context);
}

static void carrier_peer_list_changed_cb(ElaCarrier *w,
                                         const char *groupid,
                                         void *context)
{
    ElaCallbacks *cbs = ((CarrierContext*)context)->cbs;

    if (cbs && cbs->group_callbacks.peer_list_changed)
        cbs->group_callbacks.peer_list_changed(w, groupid, context);
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
    .friend_invite   = carrier_friend_invite_cb,
    .group_invite    = group_invite_cb,
    .group_callbacks = {
        .group_connected   = group_connected_cb,
        .group_message     = group_message_cb,
        .group_title       = group_title_cb,
        .peer_name         = peer_name_cb,
        .peer_list_changed = carrier_peer_list_changed_cb
    }
};

int test_suite_init_ext(TestContext *context, bool udp_disabled)
{
    CarrierContext *wctxt = context->carrier;
    char datadir[PATH_MAX];
    char logfile[PATH_MAX];
    int i = 0;

    ElaOptions opts = global_config.shared_options;
    opts.udp_enabled = !udp_disabled;
    opts.log_level = global_config.tests.loglevel;

    opts.persistent_location = datadir;
    sprintf(datadir, "%s/tests", global_config.shared_options.persistent_location);

    if (global_config.log2file) {
        opts.log_file = logfile;
        sprintf(logfile, "%s/tests/tests.log", global_config.shared_options.persistent_location);
    } else {
        opts.log_file = NULL;
    }

    wctxt->carrier = ela_new(&opts, &callbacks, wctxt);
    if (!wctxt->carrier) {
        vlogE("Error: Carrier new error (0x%x)", ela_get_error());
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
	return test_suite_init_ext(context, !global_config.shared_options.udp_enabled);
}

int test_suite_cleanup(TestContext *context)
{
    CarrierContext *wctxt = context->carrier;

    ela_kill(wctxt->carrier);
    pthread_join(wctxt->thread, 0);

    return 0;
}

int add_friend_anyway(TestContext *context, const char *userid,
                      const char *address)
{
    CarrierContext *wctxt = context->carrier;
    int rc;

    if (!ela_is_friend(wctxt->carrier, userid)) {
        rc = ela_add_friend(wctxt->carrier, address, "auto-reply");
        if (rc < 0) {
            vlogE("Error: attempt to add friend error.");
            return rc;
        }

        // wait for friend_added callback invoked.
        cond_wait(wctxt->cond);
    } else {
        char userid[ELA_MAX_ID_LEN + 1];
        char useraddr[ELA_MAX_ADDRESS_LEN + 1];
        const char *hello = "auto-reply";

        (void)ela_get_userid(wctxt->carrier, userid, sizeof(userid));
        (void)ela_get_address(wctxt->carrier, useraddr, sizeof(useraddr));

        rc = write_cmd("fadd %s %s %s\n", userid, useraddr, hello);
        CU_ASSERT_FATAL(rc > 0);
    }

    // wait until robot being notified us connected.
    char buf[2][32];
    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL_FATAL(buf[0], "fadd");
    CU_ASSERT_STRING_EQUAL_FATAL(buf[1], "succeeded");

    status_cond_wait(wctxt->friend_status_cond, ONLINE);

    return 0;
}

int remove_friend_anyway(TestContext *context, const char *userid)
{
    CarrierContext *wctxt = context->carrier;
    int rc;
    char me[ELA_MAX_ID_LEN + 1];

    if (ela_is_friend(wctxt->carrier, userid)) {
        rc = ela_remove_friend(wctxt->carrier, userid);
        if (rc < 0) {
            vlogE("Error: remove friend error (%x)", ela_get_error());
            return rc;
        }

        // wait until robot offline.
        status_cond_wait(wctxt->friend_status_cond, OFFLINE);

        // wait for friend_removed callback invoked.
        cond_wait(wctxt->cond);
    }

    (void)ela_get_userid(wctxt->carrier, me, sizeof(me));
    write_cmd("fremove %s\n", me);

    // wait for completion of robot "fremove" command.
    char buf[2][32];
    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL_FATAL(buf[0], "fremove");
    CU_ASSERT_STRING_EQUAL_FATAL(buf[1], "succeeded");

    return 0;
}

int robot_sinit(void)
{
    return write_cmd("sinit\n");
}

void robot_sfree(void)
{
    write_cmd("sfree\n");
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

    rc = ela_session_init(wctxt->carrier);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    rc = robot_sinit();
    TEST_ASSERT_TRUE(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
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

    rc = ela_session_request(sctxt->session, NULL, sctxt->request_complete_cb, sctxt);
    TEST_ASSERT_TRUE(rc == 0);

    cond_wait(stream_ctxt->cond);
    TEST_ASSERT_TRUE(stream_ctxt->state == ElaStreamState_transport_ready);
    TEST_ASSERT_TRUE(stream_ctxt->state_bits & (1 << ElaStreamState_transport_ready));

    rc = read_ack("%32s %32s", cmd, result);
    TEST_ASSERT_TRUE(rc == 2);
    TEST_ASSERT_TRUE(strcmp(cmd, "srequest") == 0);
    TEST_ASSERT_TRUE(strcmp(result, "received") == 0);

    rc = write_cmd("sreply confirm %d %d\n", stream_type, stream_options);
    TEST_ASSERT_TRUE(rc > 0);

    cond_wait(sctxt->request_complete_cond);
    TEST_ASSERT_TRUE(sctxt->request_received == 0);
    TEST_ASSERT_TRUE(sctxt->request_complete_status == 0);

    rc = read_ack("%32s %32s", cmd, result);
    TEST_ASSERT_TRUE(rc == 2);
    TEST_ASSERT_TRUE(strcmp(cmd, "sreply") == 0);
    TEST_ASSERT_TRUE(strcmp(result, "success") == 0);

    cond_wait(stream_ctxt->cond);

    if (stream_ctxt->state != ElaStreamState_connecting &&
        stream_ctxt->state != ElaStreamState_connected) {
        // if error, consume ctrl acknowlege from robot.
        read_ack("%32s %32s", cmd, result);
    }

    // Stream 'connecting' state is a transient state.
    TEST_ASSERT_TRUE(stream_ctxt->state == ElaStreamState_connecting ||
                     stream_ctxt->state == ElaStreamState_connected);
    TEST_ASSERT_TRUE(stream_ctxt->state_bits & (1 << ElaStreamState_connecting));

    rc = read_ack("%32s %32s", cmd, result);
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

void test_group_scheme(TestContext *context,
                       int (*do_work_cb)(TestContext *))
{
    CarrierContext *wctx = context->carrier;
    char cmd[32] = {0};
    char result[32] = {0};
    int rc = 0;
    int len = ELA_MAX_ID_LEN + 1;

    context->context_reset(context);

    rc = add_friend_anyway(context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctx->carrier, robotid));

    rc = ela_new_group(wctx->carrier, wctx->groupid, sizeof(wctx->groupid));
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FATAL(strlen(wctx->groupid) > 0);

    rc = ela_group_invite(wctx->carrier, wctx->groupid, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    // wait until robot having received the invitation
    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL_FATAL(cmd, "ginvite");
    CU_ASSERT_STRING_EQUAL_FATAL(result, "received");

    rc = write_cmd("gjoin\n");
    CU_ASSERT_FATAL(rc > 0);

    // wait until robot having joined in the group
    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gjoin") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait until peer_list_changed callback invoked
    cond_wait(wctx->group_cond);
    CU_ASSERT_TRUE_FATAL(strcmp(wctx->groupid, wctx->joined_groupid) == 0);
    CU_ASSERT_EQUAL_FATAL(wctx->peer_list_cnt, 1);

    rc = do_work_cb ? do_work_cb(context) : 0;
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    rc = write_cmd("gleave\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gleave") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait until peer_list_changed callback invoked
    cond_wait(wctx->group_cond);

    rc = ela_leave_group(wctx->carrier, wctx->groupid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
}

void test_filetransfer_scheme(TestContext *context, int (*do_work_cb)(TestContext *),
                              bool use_ft_info)
{
    CarrierContext *wctxt = context->carrier;
    char userid[ELA_MAX_ID_LEN + 1] = {0};
    char cmd[32] = {0};
    char result[32] = {0};
    uint8_t ft_con_state_bits = 0;
    int rc;

    context->context_reset(context);

    rc = add_friend_anyway(context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    rc = ela_filetransfer_init(wctxt->carrier, NULL, NULL);
    CU_ASSERT_EQUAL(rc, 0);

    rc = write_cmd("ft_init\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_init") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    wctxt->ft = ela_filetransfer_new(wctxt->carrier, robotid,
                                     use_ft_info ? wctxt->ft_info : NULL,
                                     wctxt->ft_cbs, context);
    CU_ASSERT_PTR_NOT_NULL(wctxt->ft);

    ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    rc = write_cmd("ft_new %s\n", userid);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_new") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    rc = ela_filetransfer_connect(wctxt->ft);
    CU_ASSERT_EQUAL(rc, 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_connect") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "received") == 0);

    rc = write_cmd("ft_accept\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_accept") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    /* Wait for the ft_state_changed_cb to be invoked. After invocation,
       the filetransfer connection state should have changed to be
       FileTransferConnection_connecting. */
    cond_wait(wctxt->ft_cond);

    /* Wait for the ft_state_changed_cb to be invoked. After invocation,
       the filetransfer connection state should have changed to be
       FileTransferConnection_connected. */
    cond_wait(wctxt->ft_cond);

    ft_con_state_bits |= 1 << FileTransferConnection_connecting;
    ft_con_state_bits |= 1 << FileTransferConnection_connected;
    CU_ASSERT_EQUAL_FATAL(wctxt->ft_con_state_bits, ft_con_state_bits);

    if (use_ft_info) {
        char file_name[ELA_MAX_FILE_NAME_LEN + 1] = {0};
        char file_id[ELA_MAX_FILE_ID_LEN + 1] = {0};
        int size;

        rc = read_ack("%64s %64s %d", file_name, file_id, &size);
        CU_ASSERT_TRUE_FATAL(rc == 3);
        CU_ASSERT_TRUE_FATAL(strcmp(file_name, wctxt->ft_info->filename) == 0);
        CU_ASSERT_TRUE_FATAL(size == wctxt->ft_info->size);
    }

    rc = do_work_cb ? do_work_cb(context) : 0;
    CU_ASSERT_EQUAL(rc, 0);

    rc = write_cmd("ft_cleanup\n");
    CU_ASSERT(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_cleanup") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    /* Wait for the ft_state_changed_cb to be invoked. After invocation,
       the filetransfer connection state should have changed to be
       FileTransferConnection_closed. */
    cond_wait(wctxt->ft_cond);

    ft_con_state_bits |= 1 << FileTransferConnection_closed;
    CU_ASSERT_EQUAL(wctxt->ft_con_state_bits, ft_con_state_bits);

    ela_filetransfer_close(wctxt->ft);
    ela_filetransfer_cleanup(wctxt->carrier);
}
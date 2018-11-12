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
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include <vlog.h>
#if defined(_WIN32) || defined(_WIN64)
#include <posix_helper.h>
#endif

#include "ela_carrier.h"

#include "cond.h"
#include "test_helper.h"

struct CarrierContextExtra {
    char* from;

    // for friend request
    ElaUserInfo info;
    char* hello;
    int len;

    ElaConnectionStatus connection_status;
};

static CarrierContextExtra extra = {
    .from   = NULL,

    .hello  = NULL,
    .len    = 0,

    .connection_status = ElaConnectionStatus_Disconnected
};

static inline void wakeup(void* context)
{
    cond_signal(((CarrierContext *)context)->cond);
}

static void ready_cb(ElaCarrier *w, void *context)
{
    cond_signal(((CarrierContext *)context)->ready_cond);
}

static void friend_added_cb(ElaCarrier *w, const ElaFriendInfo *info, void *context)
{
    wakeup(context);
    vlogD("Friend %s added.", info->user_info.userid);
}

static void friend_removed_cb(ElaCarrier *w, const char *friendid, void *context)
{
    wakeup(context);
    vlogD("Friend %s removed.\n", friendid);
}

static void friend_connection_cb(ElaCarrier *w, const char *friendid,
                                 ElaConnectionStatus status, void *context)
{
    CarrierContext *wctxt = (CarrierContext *)context;

    wctxt->extra->connection_status = status;
    wctxt->friend_status = (status == ElaConnectionStatus_Connected) ?
                           ONLINE : OFFLINE;
    cond_signal(wctxt->friend_status_cond);

    vlogD("Robot connection status changed -> %s", connection_str(status));
}

static void friend_request_cb(ElaCarrier *w, const char *userid,
                              const ElaUserInfo *info,
                              const char *hello, void* context)
{
    CarrierContextExtra *extra = ((CarrierContext *)context)->extra;

    extra->from  = strdup(userid);
    extra->hello = strdup(hello);
    memcpy(&extra->info, info, sizeof(*info));

    wakeup(context);
}

static void peer_list_changed_cb(ElaCarrier *carrier, const char *groupid,
                                 void *context)
{
    wakeup(context);
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
        .friend_request  = friend_request_cb,
        .friend_added    = friend_added_cb,
        .friend_removed  = friend_removed_cb,
        .friend_message  = NULL,
        .friend_invite   = NULL,
        .group_invite    = NULL,
        .group_callbacks = {
            .peer_list_changed = peer_list_changed_cb
        }
};

static Condition DEFINE_COND(ready_cond);
static Condition DEFINE_COND(cond);
static Condition DEFINE_COND(friend_status_cond);

static CarrierContext carrier_context = {
        .cbs = &callbacks,
        .carrier = NULL,
        .ready_cond = &ready_cond,
        .cond = &cond,
        .friend_status_cond = &friend_status_cond,
        .extra = &extra
};

static void test_context_reset(TestContext *context)
{
    cond_reset(context->carrier->cond);
}

static TestContext test_context = {
        .carrier = &carrier_context,
        .session = NULL,
        .stream  = NULL,
        .context_reset = test_context_reset
};

static void test_group(void)
{
    char groupid[ELA_MAX_ID_LEN + 1];
    CarrierContext *wctx = test_context.carrier;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctx->carrier, robotid));

    rc = ela_new_group(wctx->carrier, groupid, sizeof(groupid));
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FATAL(strlen(groupid));

    rc = ela_group_invite(wctx->carrier, groupid, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    // wait until robot having received the invitation
    char buf[2][32];
    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL_FATAL(buf[0], "ginvite");
    CU_ASSERT_STRING_EQUAL_FATAL(buf[1], "received");

    rc = write_cmd("gjoin\n");
    CU_ASSERT_FATAL(rc > 0);

    // wait until robot having joined in the group
    char cmd[32];
    char result[32];
    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gjoin") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait until peer_list_changed callback invoked
    cond_wait(wctx->cond);

    rc = ela_group_send_message(wctx->carrier, groupid,
                                "hello", strlen("hello"));
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    // wait until robot having received group message
    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gmsg") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "hello") == 0);

    rc = write_cmd("gleave\n");
    CU_ASSERT_FATAL(rc > 0);

    // wait until robot having left the group
    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gleave") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait until peer_list_changed callback invoked
    cond_wait(wctx->cond);

    rc = ela_leave_group(wctx->carrier, groupid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
}

static CU_TestInfo cases[] = {
        { "test_group",                test_group                },
        { NULL, NULL }
};

CU_TestInfo *group_test_get_cases(void)
{
    return cases;
}

int group_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int group_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

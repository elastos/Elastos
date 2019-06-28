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
#include <crystal.h>
#include <CUnit/Basic.h>

#include "ela_carrier.h"
#include "cond.h"
#include "test_helper.h"

struct CarrierContextExtra {
    char* from;
    char *gfrom;
    // for friend request
    ElaUserInfo info;
    char* hello;
    int len;

    char groupid[ELA_MAX_ID_LEN + 1];
    char gcookie[128];
    int gcookie_len;
};

static CarrierContextExtra extra = {
    .from   = NULL,
    .gfrom = NULL,
    .hello  = NULL,
    .len    = 0,

    .groupid = {0},
    .gcookie = {0},
    .gcookie_len = 0
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

static void group_invite_cb(ElaCarrier *w, const char *from,
                            const void *cookie, size_t len, void *context)
{
    CarrierContext *wctx = (CarrierContext *)context;
    CarrierContextExtra *extra = wctx->extra;

    memcpy(extra->gcookie, cookie, len);
    extra->gcookie_len = (int)len;
    if (extra->gfrom)
        free(extra->gfrom);

    extra->gfrom = strdup(from);

    cond_signal(wctx->group_cond);
}

static void group_connected_cb(ElaCarrier *carrier, const char *groupid,
                               void *context)
{
    CarrierContext *wctx = (CarrierContext *)context;

    cond_signal(wctx->group_cond);
}

static void peer_list_changed_cb(ElaCarrier *carrier, const char *groupid,
                                 void *context)
{
    CarrierContext *wctx = (CarrierContext *)context;

    wctx->peer_list_cnt++;
    strcpy(wctx->joined_groupid, groupid);

    cond_signal(wctx->group_cond);
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
    .friend_invite   = NULL,
    .group_invite    = group_invite_cb,
    .group_callbacks = {
        .group_connected = group_connected_cb,
        .group_message = NULL,
        .group_title = NULL,
        .peer_name = NULL,
        .peer_list_changed = peer_list_changed_cb,
    }
};

static Condition DEFINE_COND(ready_cond);
static Condition DEFINE_COND(cond);
static Condition DEFINE_COND(group_cond);
static StatusCondition DEFINE_STATUS_COND(friend_status_cond);

static CarrierContext carrier_context = {
    .cbs = &callbacks,
    .carrier = NULL,
    .ready_cond = &ready_cond,
    .cond = &cond,
    .friend_status_cond = &friend_status_cond,
    .group_cond = &group_cond,
    .extra = &extra
};

static void test_context_reset(TestContext *context)
{
    context->carrier->peer_list_cnt = 0;
    cond_reset(context->carrier->cond);
    cond_reset(context->carrier->group_cond);
    status_cond_reset(context->carrier->friend_status_cond);
}

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = NULL,
    .stream  = NULL,
    .context_reset = test_context_reset
};

static int group_invite_after_joining_cb(TestContext *ctx)
{
    CarrierContext *wctx = test_context.carrier;
    const char *msg = "hello";
    char cmd[32] = {0};
    char result[32] = {0};
    int rc;

    rc = ela_group_invite(wctx->carrier, wctx->groupid, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    rc = ela_group_send_message(wctx->carrier, wctx->groupid,
                                msg, strlen(msg));
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    // wait until robot having received group message
    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gmsg") == 0);
    CU_ASSERT_TRUE_FATAL(strncmp(result, msg, strlen(msg)) == 0);

    return 0;
}

static int group_join_twice_cb(TestContext *ctx)
{
    int rc;
    char cmd[32];
    char result[32];

    rc = write_cmd("gjoin\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gjoin") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "failed") == 0);

    return 0;
}

static int group_leave_then_join_cb(TestContext *ctx)
{
    CarrierContext *wctx = test_context.carrier;
    char cmd[32];
    char result[32];
    int rc;

    rc = write_cmd("gleave\n");
    CU_ASSERT_FATAL(rc > 0);

    // wait until robot having left the group
    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gleave") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait until peer_list_changed callback invoked
    cond_wait(wctx->group_cond);

    rc = ela_group_invite(wctx->carrier, wctx->groupid, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL_FATAL(cmd, "ginvite");
    CU_ASSERT_STRING_EQUAL_FATAL(result, "received");

    rc = write_cmd("gjoin\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gjoin") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait until peer_list_changed callback invoked
    cond_wait(wctx->group_cond);

    return 0;
}

static void test_group_invite(void)
{
    test_group_scheme(&test_context, NULL);
}

static void test_group_invite_twice(void)
{
    test_group_scheme(&test_context, group_invite_after_joining_cb);
}

static void test_group_join(void)
{
    CarrierContext *wctx = test_context.carrier;
    CarrierContextExtra *extra = wctx->extra;
    char userid[ELA_MAX_ID_LEN + 1] = {0};
    char useraddr[ELA_MAX_ADDRESS_LEN + 1] = {0};
    const char *hello = "hello";
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctx->carrier, robotid));

    char cmd[32];
    char result[32];
    ela_get_userid(wctx->carrier, userid, sizeof(userid));
    rc = write_cmd("ginvite %s\n", userid);
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ginvite") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    cond_wait(wctx->group_cond);
    CU_ASSERT_TRUE_FATAL(strcmp(extra->gfrom, robotid) == 0);
    FREE_ANYWAY(extra->gfrom);
    rc = ela_group_join(wctx->carrier, robotid, extra->gcookie,
                        extra->gcookie_len, extra->groupid,
                        sizeof(extra->groupid));
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    /* wait for the peer_list_changed_cb(because of the robot's joining),
       group_connected_cb, peer_list_changed_cb(because of the cases's joining)
       callback functions to be invoked. */
    cond_wait(wctx->group_cond);
    cond_wait(wctx->group_cond);
    cond_wait(wctx->group_cond);

    rc = write_cmd("gleave\n");
    CU_ASSERT_FATAL(rc > 0);

    // wait until robot having left the group
    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gleave") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait until peer_list_changed callback invoked
    cond_wait(wctx->group_cond);

    rc = ela_leave_group(wctx->carrier, extra->groupid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
}

static void test_group_invite_stranger(void)
{
    CarrierContext *wctx = test_context.carrier;
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    int rc;

    //TO make robot as stranger.
    test_context.context_reset(&test_context);

    rc = remove_friend_anyway(&test_context, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FALSE_FATAL(ela_is_friend(wctx->carrier, robotid));

    rc = ela_new_group(wctx->carrier, groupid, sizeof(groupid));
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FATAL(strlen(groupid) > 0);

    rc = ela_group_invite(wctx->carrier, groupid, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));

    rc = ela_leave_group(wctx->carrier, groupid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
}

static void test_group_invite_myself(void)
{
    CarrierContext *wctx = test_context.carrier;
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    char userid[ELA_MAX_ID_LEN + 1] = {0};
    int rc;

    rc = ela_new_group(wctx->carrier, groupid, sizeof(groupid));
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FATAL(strlen(groupid) > 0);

    ela_get_userid(wctx->carrier, userid, sizeof(userid));
    rc = ela_group_invite(wctx->carrier, groupid, userid);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));

    rc = ela_leave_group(wctx->carrier, groupid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
}

static void test_group_join_twice(void)
{
    test_group_scheme(&test_context, group_join_twice_cb);
}

static void test_group_leave_then_join(void)
{
    test_group_scheme(&test_context, group_leave_then_join_cb);
}

static CU_TestInfo cases[] = {
    { "test_group_invite_join",         test_group_invite           },
    { "test_group_invite_twice",        test_group_invite_twice     },
    { "test_group_invite_stranger",     test_group_invite_stranger  },
    { "test_group_invite_myself",       test_group_invite_myself    },
    { "test_group_join",                test_group_join             },
    { "test_group_join_twice",          test_group_join_twice       },
    { "test_group_leave_then_join",     test_group_leave_then_join  },
    { NULL, NULL }
};

CU_TestInfo *group_invite_join_test_get_cases(void)
{
    return cases;
}

int group_invite_join_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int group_invite_join_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

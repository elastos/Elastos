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
    char *msg;
    size_t msglen;
};

static CarrierContextExtra extra = {
    .from   = NULL,
    .msg    = NULL,
    .msglen = 0,
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

static void peer_list_changed_cb(ElaCarrier *carrier, const char *groupid,
                                 void *context)
{
    CarrierContext *wctx = (CarrierContext *)context;

    wctx->peer_list_cnt++;
    strcpy(wctx->joined_groupid, groupid);

    cond_signal(wctx->group_cond);
}

static void group_message_cb(ElaCarrier *carrier, const char *groupid, const char *from,
                             const void *message, size_t length, void *context)
{
    CarrierContext *wctx = (CarrierContext *)context;
    CarrierContextExtra *extra = wctx->extra;

    extra->from  = strdup(from);
    extra->msg   = strdup(message);
    extra->msglen = length;

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
    .group_invite    = NULL,
    .group_callbacks = {
        .group_connected = NULL,
        .group_message = group_message_cb,
        .group_title = NULL,
        .peer_name = NULL,
        .peer_list_changed = peer_list_changed_cb
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
}

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = NULL,
    .stream = NULL,
    .context_reset = test_context_reset
};

static int group_message_routine(TestContext *ctx)
{
    CarrierContext *wctx = test_context.carrier;
    CarrierContextExtra *extra = wctx->extra;
    char userid[ELA_MAX_ID_LEN + 1] = {0};
    const char *msg = "hello";
    char cmd[32] = {0};
    char result[32] = {0};
    int rc;

    rc = ela_group_send_message(wctx->carrier, wctx->groupid,
                                msg, strlen(msg));
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    // wait until robot having received group message
    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gmsg") == 0);
    CU_ASSERT_TRUE_FATAL(strncmp(result, msg, strlen(msg)) == 0);

    cond_wait(wctx->group_cond);
    ela_get_userid(wctx->carrier, userid, sizeof(userid));
    CU_ASSERT_TRUE_FATAL(strcmp(extra->from, userid) == 0);
    CU_ASSERT_TRUE_FATAL(strncmp(extra->msg, msg, strlen(msg)) == 0);
    CU_ASSERT_EQUAL_FATAL(extra->msglen, strlen(msg));
    FREE_ANYWAY(extra->from);
    FREE_ANYWAY(extra->msg);

    return 0;
}

static int persistent_group_message_routine(TestContext *ctx)
{
    CarrierContext *wctx = test_context.carrier;
    CarrierContextExtra *extra = wctx->extra;
    char userid[ELA_MAX_ID_LEN + 1] = {0};
    const char *msg = "hello";
    char cmd[32] = {0};
    char result[32] = {0};
    char buf[2][32] = {0};
    char ack[128] = {0};
    char robot_id[ELA_MAX_ID_LEN + 1] = {0};
    char robot_addr[ELA_MAX_ADDRESS_LEN + 1] = {0};
    int rc;

    rc = write_cmd("killnode\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "killnode");
    CU_ASSERT_STRING_EQUAL(buf[1], "success");

    // wait until peer_list_changed callback invoked
    cond_wait(wctx->group_cond);

    status_cond_wait(wctx->friend_status_cond, OFFLINE);

    write_cmd("restartnode\n");

    rc = read_ack("%32s %45s %52s", ack, robot_id, robot_addr);
    CU_ASSERT_EQUAL(rc, 3);
    CU_ASSERT_STRING_EQUAL(ack, "ready");
    CU_ASSERT_STRING_EQUAL(robot_id, robotid);
    CU_ASSERT_STRING_EQUAL(robot_addr, robotaddr);

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "restartnode");
    CU_ASSERT_STRING_EQUAL(buf[1], "success");

    status_cond_wait(wctx->friend_status_cond, ONLINE);

    // wait until peer_list_changed callback invoked
    cond_wait(wctx->group_cond);
    sleep(1);

    rc = ela_group_send_message(wctx->carrier, wctx->groupid,
                                msg, strlen(msg));
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    // wait until robot having received group message
    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "gmsg") == 0);
    CU_ASSERT_TRUE_FATAL(strncmp(result, msg, strlen(msg)) == 0);

    cond_wait(wctx->group_cond);
    ela_get_userid(wctx->carrier, userid, sizeof(userid));
    CU_ASSERT_TRUE_FATAL(strcmp(extra->from, userid) == 0);
    CU_ASSERT_TRUE_FATAL(strncmp(extra->msg, msg, strlen(msg)) == 0);
    CU_ASSERT_EQUAL_FATAL(extra->msglen, strlen(msg));
    FREE_ANYWAY(extra->from);
    FREE_ANYWAY(extra->msg);

    return 0;
}

static void test_group_message(void)
{
    test_context.context_reset(&test_context);
    test_group_scheme(&test_context, group_message_routine);
}

static void test_persistent_group_message(void)
{
    test_context.context_reset(&test_context);
    test_group_scheme(&test_context, persistent_group_message_routine);
}

static void test_group_message_to_myself(void)
{
    CarrierContext *wctx = test_context.carrier;
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    int rc;

    rc = ela_new_group(wctx->carrier, groupid, sizeof(groupid));
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FATAL(strlen(groupid) > 0);

    rc = ela_group_send_message(wctx->carrier, groupid, "hello",
                                strlen("hello"));
    CU_ASSERT_EQUAL_FATAL(rc, -1);

    rc = ela_leave_group(wctx->carrier, groupid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
}

static CU_TestInfo cases[] = {
    { "test_group_message",             test_group_message },
    { "test_group_message_to_myself",   test_group_message_to_myself },
    { "test_persistent_group_message ", test_persistent_group_message },
    { NULL, NULL }
};

CU_TestInfo *group_message_test_get_cases(void)
{
    return cases;
}

int group_message_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int group_message_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

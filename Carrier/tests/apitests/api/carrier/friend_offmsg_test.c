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
#include <crystal.h>

#include "ela_carrier.h"

#include "cond.h"
#include "test_helper.h"

struct CarrierContextExtra {
    char* from;
    char* msg;
    int len;
};

static CarrierContextExtra extra = {
    .from = NULL,
    .msg  = NULL,
    .len  = 0
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

static void friend_message_cb(ElaCarrier *w, const char *from, const void *msg, size_t len,
                              int64_t timestamp, bool is_offline, void *context)
{
    CarrierContextExtra *extra = ((CarrierContext *)context)->extra;

    extra->from = strdup(from);
    extra->msg  = strdup((const char *)msg);
    extra->len  = (int)len;

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
    .friend_request  = NULL,
    .friend_added    = friend_added_cb,
    .friend_removed  = friend_removed_cb,
    .friend_message  = friend_message_cb,
    .friend_invite   = NULL
};

static Condition DEFINE_COND(ready_cond);
static Condition DEFINE_COND(cond);
static StatusCondition DEFINE_STATUS_COND(friend_status_cond);

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
    status_cond_reset(context->carrier->friend_status_cond);
}

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = NULL,
    .stream  = NULL,
    .context_reset = test_context_reset
};

static void send_offmsg_to_friend(int count, int timeout)
{
    CarrierContext *wctxt = test_context.carrier;
    char prefix[32] = {0};
    char buf[2][32] = {0};
    char ack[32] = {0};
    char out[32] = {0};
    bool offline = false;
    int rc;
    int i;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    rc = write_cmd("killnode\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "killnode");
    CU_ASSERT_STRING_EQUAL(buf[1], "success");

    status_cond_wait(wctxt->friend_status_cond, OFFLINE);

    sprintf(prefix, "%ld:", time(NULL));
    rc = write_cmd("offmsgprefix %s\n", prefix);
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "offmsgprefix");
    CU_ASSERT_STRING_EQUAL(buf[1], "success");

    for (i = 0; i < count; i++) {
        memset(out, 0, sizeof(out));
        sprintf(out, "%s%d", prefix, (count > 1) ? (i + 1) : i);
        rc = ela_send_friend_message(wctxt->carrier, robotid, out, strlen(out), &offline);
        CU_ASSERT_EQUAL_FATAL(rc, 0);
        CU_ASSERT_EQUAL(offline, true);
    }

    usleep(5000000);

    if (count > 1)
        rc = write_cmd("restartnode %d %d\n", timeout, count);
    else
        rc = write_cmd("restartnode %d\n", timeout);
    CU_ASSERT_TRUE_FATAL(rc > 0);

    // in offmsg casd,  robot will not ack "ready" to testcase,
    // directly wating for friend connection.
    status_cond_wait(wctxt->friend_status_cond, ONLINE);

    if (count > 1) {
        int recv_count = 0;
        rc = read_ack("%s %d", buf[0], &recv_count);
        CU_ASSERT_EQUAL(rc, 2);
        CU_ASSERT_STRING_EQUAL(buf[0], "offmsg");
        CU_ASSERT_EQUAL(count, recv_count);
    } else {
        rc = read_ack("%32s %32s", buf[0], buf[1]);
        CU_ASSERT_EQUAL(rc, 2);
        CU_ASSERT_STRING_EQUAL(buf[0], "offmsg");
        CU_ASSERT_STRING_EQUAL(buf[1], out);
    }
}

static void test_send_offmsg_to_friend(void)
{
    send_offmsg_to_friend(1, 900);
}

static void test_send_multi_offmsgs_to_friend(void)
{
    send_offmsg_to_friend(10, 900);
}

static CU_TestInfo cases[] = {
    { "test_send_offmsg_to_friend",   test_send_offmsg_to_friend  },
    { "test_send_multi_offmsgs_to_friend",  test_send_multi_offmsgs_to_friend },
    {NULL, NULL }
};

CU_TestInfo *friend_offmsg_test_get_cases(void)
{
    return cases;
}

int friend_offmsg_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int friend_offmsg_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

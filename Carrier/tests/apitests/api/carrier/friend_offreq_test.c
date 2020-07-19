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
    ElaConnectionStatus connection_status;
};

static CarrierContextExtra extra = {
    .from = NULL,
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

static
void friend_added_cb(ElaCarrier *w, const ElaFriendInfo *info, void *context)
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
    status_cond_signal(wctxt->friend_status_cond, status);

    vlogD("Robot connection status changed -> %s", connection_str(status));
}

static void kill_node()
{
    int rc;
    char buf[2][32] = {0};

    clear_socket_buffer();
    rc = write_cmd("killnode\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "killnode");
    CU_ASSERT_STRING_EQUAL(buf[1], "success");
}

static void start_node()
{
    int rc;
    char buf[2][32] = {0};

    clear_socket_buffer();
    rc = write_cmd("restartnode %d\n", 10000);
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s", buf[0]);
    CU_ASSERT_EQUAL_FATAL(rc, 1);
    CU_ASSERT_STRING_EQUAL_FATAL(buf[0], "ready");
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

static void test_send_offline_friend_request(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char userid[ELA_MAX_ID_LEN + 1];
    char prefix[32] = {0};
    char hello[32] = {0};
    char buf[2][32] = {0};
    int rc;

    test_context.context_reset(&test_context);

    rc = remove_friend_anyway(&test_context, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FALSE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    kill_node();
    status_cond_wait(wctxt->friend_status_cond, OFFLINE);

    sprintf(prefix, "%ld", time(NULL));
    rc = write_cmd("offmsgprefix %s\n", prefix);
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "offmsgprefix");
    CU_ASSERT_STRING_EQUAL(buf[1], "success");

    sprintf(hello, "%s:greeting", prefix);
    rc = ela_add_friend(wctxt->carrier, robotaddr, hello);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    cond_wait(wctxt->cond);

    rc = write_cmd("restartnode %d\n", 900);
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s", buf[0]);
    CU_ASSERT_EQUAL_FATAL(rc, 1);
    CU_ASSERT_STRING_EQUAL_FATAL(buf[0], "ready");

    // wait until robot having received "fadd” request.
    rc = read_ack("%32s %32s", buf[0], buf[1]);

    vlogD("<%s> buf[0]: %s, buf[1]: %s", __FUNCTION__, buf[0], buf[1]);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL_FATAL(buf[0], "offreq");
    CU_ASSERT_STRING_EQUAL_FATAL(buf[1], hello);

    ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    rc = write_cmd("faccept %s\n", userid);
    CU_ASSERT_FATAL(rc > 0);

    // wait for friend_added() callback to be invoked.
    cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(ela_is_friend(wctxt->carrier, robotid));
    // wait for friend connection (online) callback to be invoked.
    status_cond_wait(wctxt->friend_status_cond, ONLINE);
    CU_ASSERT_TRUE(extra->connection_status == ElaConnectionStatus_Connected);

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "fadd");
    CU_ASSERT_STRING_EQUAL(buf[1], "succeeded");

    /*

    // ensure filter old offline request message
    kill_node();
    status_cond_wait(wctxt->friend_status_cond, OFFLINE);
    start_node();
    status_cond_wait(wctxt->friend_status_cond, ONLINE);

    rc = remove_friend_anyway(&test_context, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FALSE_FATAL(ela_is_friend(wctxt->carrier, robotid));
    usleep(2000000);

    kill_node();
    status_cond_wait(wctxt->friend_status_cond, OFFLINE);

    rc = ela_add_friend(wctxt->carrier, robotaddr, "auto-reply");
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    cond_wait(wctxt->cond);

    start_node();

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL_FATAL(buf[0], "fadd");
    CU_ASSERT_STRING_EQUAL_FATAL(buf[1], "succeeded");
    status_cond_wait(wctxt->friend_status_cond, ONLINE);

    */
}

static CU_TestInfo cases[] = {
    { "test_send_offline_friend_request", test_send_offline_friend_request },
    {NULL, NULL }
};

CU_TestInfo *friend_offreq_test_get_cases(void)
{
    return cases;
}

int friend_offreq_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int friend_offreq_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

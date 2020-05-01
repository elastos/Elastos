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

static void test_send_message_to_friend(void)
{
    CarrierContext *wctxt = test_context.carrier;
    bool is_offline;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    const char* out = "message-test";
    rc = ela_send_friend_message(wctxt->carrier, robotid, out, strlen(out), &is_offline);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_EQUAL_FATAL(is_offline, false);

    char in[64];
    rc = read_ack("%64s", in);
    CU_ASSERT_EQUAL(rc, 1);
    CU_ASSERT_STRING_EQUAL(in, out);
}

static void test_send_message_from_friend(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char userid[ELA_MAX_ID_LEN + 1];
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    const char* msg = "message-test";

    rc = write_cmd("fmsg %s %s\n", userid, msg);
    CU_ASSERT_FATAL(rc > 0);

    // wait for message from robot.
    bool bRet = cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(bRet);
    if (bRet) {
        CU_ASSERT_NSTRING_EQUAL(extra->from, robotid, strlen(robotid));
        CU_ASSERT_NSTRING_EQUAL(extra->msg, msg, extra->len);
        CU_ASSERT_EQUAL(extra->len, strlen(msg));

        FREE_ANYWAY(extra->from);
        FREE_ANYWAY(extra->msg);
    }
}

static void test_send_message_to_stranger(void)
{
    CarrierContext *wctxt = test_context.carrier;
    int rc;

    test_context.context_reset(&test_context);

    rc = remove_friend_anyway(&test_context, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FALSE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    const char* msg = "test-message";
    rc = ela_send_friend_message(wctxt->carrier, robotid, msg, strlen(msg), NULL);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
}

static void test_send_message_to_self(void)
{
    CarrierContext *wctxt = test_context.carrier;
    char userid[ELA_MAX_ID_LEN + 1];
    char nodeid[ELA_MAX_ID_LEN + 1];
    const char* msg = "test-message";
    int rc;

    test_context.context_reset(&test_context);

    (void)ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    (void)ela_get_nodeid(wctxt->carrier, nodeid, sizeof(nodeid));
    rc = ela_send_friend_message(wctxt->carrier, userid, msg, strlen(msg), NULL);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL_FATAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static void test_send_big_message_to_friend(void)
{
    CarrierContext *wctxt = test_context.carrier;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    static char out[(ELA_MAX_APP_MESSAGE_LEN << 1) + 1];
    size_t outsz = sizeof(out) / sizeof(out[0]);
    char outchar = 'l';
    memset(out, outchar, outsz - 1);
    out[outsz - 1] = '\0';

    rc = ela_send_friend_message(wctxt->carrier, robotid, out, strlen(out), NULL);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    char in[(ELA_MAX_APP_MESSAGE_LEN << 1) + 1];
    size_t insz = sizeof(in) / sizeof(in[0]);
    rc = read_ack("%2049s", in);
    CU_ASSERT_EQUAL(rc, 1);
    CU_ASSERT_TRUE(!memcmp(out, in, insz - 1));

    rc = ela_send_friend_message(wctxt->carrier, robotid, out,
                                 ELA_MAX_APP_MESSAGE_LEN - 1, NULL);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    memset(in, 0, sizeof(in));
    rc = read_ack("%1024s", in);
    CU_ASSERT_EQUAL(rc, 1);
    CU_ASSERT_TRUE(!memcmp(out, in, ELA_MAX_APP_MESSAGE_LEN - 1));
}

static CU_TestInfo cases[] = {
    { "test_send_message_to_friend",     test_send_message_to_friend },
    { "test_send_message_from_friend",   test_send_message_from_friend },
    { "test_send_message_to_stranger",   test_send_message_to_stranger },
    { "test_send_message_to_self",       test_send_message_to_self },
    { "test_send_big_message_to_friend", test_send_big_message_to_friend },
    {NULL, NULL }
};

CU_TestInfo *friend_message_test_get_cases(void)
{
    return cases;
}

int friend_message_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int friend_message_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

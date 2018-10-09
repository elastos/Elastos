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
#include <vlog.h>

#include "ela_carrier.h"

#include "cond.h"
#include "test_helper.h"

struct CarrierContextExtra {
    char *from;
    char *reason;
    char *data;
    int len;
    int status;
};

static CarrierContextExtra extra = {
    .from   = NULL,
    .reason = NULL,
    .data   = NULL,
    .len    = 0,
    .status = -1
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

    wakeup(context);
    wctxt->robot_online = (status == ElaConnectionStatus_Connected);

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

static Condition DEFINE_COND(ready_cond);
static Condition DEFINE_COND(cond);

static CarrierContext carrier_context = {
    .cbs = &callbacks,
    .carrier = NULL,
    .ready_cond = &ready_cond,
    .cond = &cond,
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

static void friend_invite_response_cb(ElaCarrier *w, const char *from, int status,
                                      const char *reason, const void *content, size_t len,
                                      void *context)
{
    CarrierContextExtra *extra = ((CarrierContext *)context)->extra;

    extra->from   = strdup(from);
    extra->status = status;
    extra->reason = (status != 0) ? (reason  ? strdup(reason ) : NULL) : NULL;
    extra->data   = (status == 0) ? (content ? strdup((const char *)content) : NULL) : NULL;
    extra->len    = (status == 0) ? (int)len : 0;

    wakeup(context);
}

static void test_friend_invite_confirm(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char userid[ELA_MAX_ID_LEN + 1];
    char to[ELA_MAX_ID_LEN * 2 + 1];
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    const char* hello = "hello";
    rc = ela_invite_friend(wctxt->carrier, robotid, hello,
                               strlen(hello) + 1,
                               friend_invite_response_cb, wctxt);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    char in[32] = {0};
    char in2[32] = {0};
    rc = read_ack("%32s %32s", in, in2);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(in, "data");
    CU_ASSERT_STRING_EQUAL(in2, hello);

    (void)ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    sprintf(to, "%s", userid);
    const char* invite_rsp_data = "invitation-confirmed";
    rc = write_cmd("freplyinvite %s confirm %s\n", to, invite_rsp_data);
    CU_ASSERT_FATAL(rc > 0);

    // wait for invite response callback invoked.
    bool bRet = cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(bRet);
    if (bRet) {
        CU_ASSERT_NSTRING_EQUAL(extra->from, robotid, strlen(robotid));
        CU_ASSERT_EQUAL(extra->status, 0);
        CU_ASSERT_PTR_NULL(extra->reason);
        CU_ASSERT_STRING_EQUAL(extra->data, invite_rsp_data);
        CU_ASSERT_EQUAL(strlen(extra->data), strlen(invite_rsp_data));
        CU_ASSERT_EQUAL(extra->len, strlen(invite_rsp_data) + 1);

        FREE_ANYWAY(extra->from);
        FREE_ANYWAY(extra->data);
    }
}

static void test_friend_invite_reject(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char userid[ELA_MAX_ID_LEN + 1];
    char to[ELA_MAX_ID_LEN * 2 + 1];
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    const char* hello = "hello";
    rc = ela_invite_friend(wctxt->carrier, robotid, hello, strlen(hello),
                               friend_invite_response_cb, wctxt);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    char in[32] = {0};
    char in2[32] = {0};
    rc = read_ack("%32s %32s", in, in2);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(in, "data");
    CU_ASSERT_STRING_EQUAL(in2, hello);

    (void)ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    sprintf(to, "%s", userid);

    const char* reason = "unknown-error";
    rc = write_cmd("freplyinvite %s refuse %s\n", to, reason);
    CU_ASSERT_FATAL(rc > 0);

    // wait for invite response callback invoked.
    bool bRet = cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(bRet);
    if (bRet) {
        CU_ASSERT_NSTRING_EQUAL(extra->from, robotid, strlen(robotid));
        CU_ASSERT(extra->status != 0);
        CU_ASSERT_STRING_EQUAL(extra->reason, reason);
        CU_ASSERT_PTR_NULL(extra->data);

        FREE_ANYWAY(extra->from);
        FREE_ANYWAY(extra->reason);
    }
}

static void test_friend_invite_stranger(void)
{
    CarrierContext *wctxt = test_context.carrier;
    const char* hello = "hello";
    int rc;

    test_context.context_reset(&test_context);

    rc = remove_friend_anyway(&test_context, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FALSE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    rc = ela_invite_friend(wctxt->carrier, robotid, hello, strlen(hello),
                               friend_invite_response_cb, wctxt);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
}

static void test_friend_invite_self(void)
{
    CarrierContext *wctxt = test_context.carrier;
    char userid[ELA_MAX_ID_LEN + 1];
    const char* hello = "hello";
    int rc;

    (void)ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    rc = ela_invite_friend(wctxt->carrier, userid, hello, strlen(hello),
                               friend_invite_response_cb, wctxt);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
}

static CU_TestInfo cases[] = {
    { "test_friend_invite_confirm",  test_friend_invite_confirm },
    { "test_friend_invite_reject",   test_friend_invite_reject },
    { "test_friend_invite_stranger", test_friend_invite_stranger },
    { "test_friend_invite_self",     test_friend_invite_self },
    { NULL, NULL }
};

CU_TestInfo *friend_invite_test_get_cases(void)
{
    return cases;
}

int friend_invite_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    if (ela_is_friend(test_context.carrier->carrier, robotid)) {
        // wait for robot online.
        cond_wait(test_context.carrier->cond);
    }

    return 0;
}

int friend_invite_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

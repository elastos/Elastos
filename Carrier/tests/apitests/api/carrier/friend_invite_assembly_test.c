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

#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h>
#endif

#include <CUnit/Basic.h>
#include <crystal.h>

#include "ela_carrier.h"

#include "cond.h"
#include "test_helper.h"

struct CarrierContextExtra {
    char *from;
    char *bundle;
    char *reason;
    char *data;
    int len;
    int status;
};

static CarrierContextExtra extra = {
    .from   = NULL,
    .bundle = NULL,
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

static void friend_invite_response_cb(ElaCarrier *w, const char *from, const char *bundle,
                                      int status, const char *reason, const void *content,
                                      size_t len, void *context)
{
    CarrierContextExtra *extra = ((CarrierContext *)context)->extra;

    extra->from   = strdup(from);
    extra->bundle = bundle ? strdup(bundle) : NULL;
    extra->status = status;
    extra->reason = (status != 0) ? strdup(reason) : NULL;
    extra->len    = (int)len;
    if (content && len > 0) {
        extra->data = calloc(1, len);
        memcpy(extra->data, content, len);
    } else {
        extra->data = NULL;
    }

    wakeup(context);
}

static void test_friend_invite_assembly_confirm(int hello_len, const char *bundle)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char userid[ELA_MAX_ID_LEN + 1];
    char *hello = calloc(1, hello_len);
    int rc;
    bool is_wakeup;

    if (!hello)
        return;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    memset(hello, 'H', hello_len);
    rc = ela_invite_friend(wctxt->carrier, robotid, bundle, hello, hello_len,
                           friend_invite_response_cb, wctxt);
    if (bundle && (strlen(bundle) == 0 || strlen(bundle) > ELA_MAX_BUNDLE_LEN)) {
        CU_ASSERT_EQUAL(rc, -1);
        CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return;
    } else {
        CU_ASSERT_EQUAL_FATAL(rc, 0);
    }

    char val[2][32] = {{0}, {0}};
    int len = 0;
    rc = read_ack("%32s %32s %d", val[0], val[1], &len);
    CU_ASSERT_EQUAL_FATAL(rc, 3);
    CU_ASSERT_STRING_EQUAL(val[0], "data");
    CU_ASSERT_STRING_EQUAL(val[1], "bigdata");
    CU_ASSERT_EQUAL_FATAL(len, hello_len);

    (void)ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    rc = write_cmd("freplyinvite_bigdata %s confirm\n", userid);
    CU_ASSERT_FATAL(rc > 0);

    // wait for invite response callback invoked.
    is_wakeup = cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(is_wakeup);
    if (is_wakeup) {
        CU_ASSERT_NSTRING_EQUAL(extra->from, robotid, strlen(robotid));
        if (bundle) {
            CU_ASSERT_NSTRING_EQUAL(extra->bundle, bundle, strlen(bundle));
        } else {
            CU_ASSERT_PTR_NULL(extra->bundle);
        }
        CU_ASSERT_EQUAL(extra->status, 0);
        CU_ASSERT_PTR_NULL(extra->reason);
        CU_ASSERT_TRUE(memcmp(extra->data, hello, hello_len) == 0);
        CU_ASSERT_EQUAL(extra->len, hello_len);

        FREE_ANYWAY(extra->from);
        FREE_ANYWAY(extra->bundle);
        FREE_ANYWAY(extra->data);
    }
    FREE_ANYWAY(hello);
}

static void test_friend_invite_assembly_confirm_d1200_b0(void)
{
    test_friend_invite_assembly_confirm(1200, "");
}

static void test_friend_invite_assembly_confirm_d1200_bnull(void)
{
    test_friend_invite_assembly_confirm(1200, NULL);
}

static void test_friend_invite_assembly_confirm_d1200_b20(void)
{
    char bundle[20] = {0};
    memset(bundle, 'B', sizeof(bundle) - 1);
    test_friend_invite_assembly_confirm(1200, bundle);
}

static void test_friend_invite_assembly_confirm_d1270_b20(void)
{
    char bundle[20] = {0};
    memset(bundle, 'B', sizeof(bundle) - 1);
    test_friend_invite_assembly_confirm(1270, bundle);
}

static void test_friend_invite_assembly_confirm_d2550_b20(void)
{
    char bundle[20] = {0};
    memset(bundle, 'B', sizeof(bundle) - 1);
    test_friend_invite_assembly_confirm(2550, bundle);
}

static void test_friend_invite_assembly_confirm_d2550_bmaxlen(void)
{
    char bundle[ELA_MAX_BUNDLE_LEN + 1] = {0};
    memset(bundle, 'B', sizeof(bundle) - 1);
    test_friend_invite_assembly_confirm(2550, bundle);
}

static void test_friend_invite_assembly_confirm_dmaxlen_bnull(void)
{
    test_friend_invite_assembly_confirm(ELA_MAX_INVITE_DATA_LEN, NULL);
}

static void test_friend_invite_assembly_reject_base(const char *bundle, const char *reason)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char userid[ELA_MAX_ID_LEN + 1];
    char *hello = "hello";
    int rc;
    bool is_wakup;

    CU_ASSERT_PTR_NOT_NULL(hello);
    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    rc = ela_invite_friend(wctxt->carrier, robotid, bundle, hello, strlen(hello) + 1,
                           friend_invite_response_cb, wctxt);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    char val[2][32] = {{0},{0}};
    rc = read_ack("%32s %32s", val[0], val[1]);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(val[0], "data");
    CU_ASSERT_STRING_EQUAL(val[1], hello);

    (void)ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    rc = write_cmd("freplyinvite_bigdata %s refuse %s\n", userid, reason);
    CU_ASSERT_FATAL(rc > 0);

    // wait for invite response callback invoked.
    is_wakup = cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(is_wakup);
    if (is_wakup) {
        CU_ASSERT_NSTRING_EQUAL(extra->from, robotid, strlen(robotid));
        CU_ASSERT(extra->status != 0);
        if (reason) {
            CU_ASSERT_NSTRING_EQUAL(extra->reason, reason, strlen(reason));
        } else {
            CU_ASSERT_STRING_EQUAL(extra->reason, "(null)");
        }

        FREE_ANYWAY(extra->from);
        FREE_ANYWAY(extra->reason);
    }
}

static void test_friend_invite_assembly_reject_b20_r20(void)
{
    char bundle[20] = {0};
    char reason[20] = {0};
    memset(bundle, 'B', sizeof(bundle) - 1);
    memset(reason, 'R', sizeof(reason) - 1);
    test_friend_invite_assembly_reject_base(bundle, reason);
}

static void test_friend_invite_assembly_reject_bnull_r20(void)
{
    char reason[20] = {0};
    memset(reason, 'B', sizeof(reason) - 1);
    test_friend_invite_assembly_reject_base(NULL, reason);
}

static void test_friend_invite_assembly_reject_bnull_rmaxlen(void)
{
    char reason[ELA_MAX_INVITE_REPLY_REASON_LEN + 1] = {0};
    memset(reason, 'R', sizeof(reason) - 1);
    test_friend_invite_assembly_reject_base(NULL, reason);
}

static void test_friend_invite_assembly_reject_bmaxlen_rmaxlen(void)
{
    char bundle[ELA_MAX_BUNDLE_LEN + 1] = {0};
    char reason[ELA_MAX_INVITE_REPLY_REASON_LEN + 1] = {0};
    memset(bundle, 'B', sizeof(bundle) - 1);
    memset(reason, 'R', sizeof(reason) - 1);
    test_friend_invite_assembly_reject_base(bundle, reason);
}

static void test_friend_invite_with_overlong_data(void)
{
    CarrierContext *wctxt = test_context.carrier;
    char data[ELA_MAX_INVITE_DATA_LEN + 1] = {0};
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    memset(data, 'T', ELA_MAX_INVITE_DATA_LEN);
    rc = ela_invite_friend(wctxt->carrier, NULL, robotid, data, sizeof(data),
                           friend_invite_response_cb, wctxt);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static CU_TestInfo cases[] = {
    { "test_friend_invite_assembly_confirm_d1200_b0",       test_friend_invite_assembly_confirm_d1200_b0 },
    { "test_friend_invite_assembly_confirm_d1200_bnull",    test_friend_invite_assembly_confirm_d1200_bnull },
    { "test_friend_invite_assembly_confirm_d1200_b20",      test_friend_invite_assembly_confirm_d1200_b20},
    { "test_friend_invite_assembly_confirm_d1270_b20",      test_friend_invite_assembly_confirm_d1270_b20},
    { "test_friend_invite_assembly_confirm_d2550_b20",      test_friend_invite_assembly_confirm_d2550_b20},
    { "test_friend_invite_assembly_confirm_d2550_bmaxlen",  test_friend_invite_assembly_confirm_d2550_bmaxlen},
    { "test_friend_invite_assembly_confirm_dmaxlen_bnull",  test_friend_invite_assembly_confirm_dmaxlen_bnull },
    { "test_friend_invite_assembly_reject_b20_r20",         test_friend_invite_assembly_reject_b20_r20 },
    { "test_friend_invite_assembly_reject_bnull_r20",       test_friend_invite_assembly_reject_bnull_r20 },
    { "test_friend_invite_assembly_reject_bnull_rmaxlen",   test_friend_invite_assembly_reject_bnull_rmaxlen },
    { "test_friend_invite_assembly_reject_bmaxlen_rmaxlen", test_friend_invite_assembly_reject_bmaxlen_rmaxlen},
    { "test_friend_invite_with_overlong_data",              test_friend_invite_with_overlong_data },
    { NULL, NULL }
};

CU_TestInfo *friend_invite_assembly_test_get_cases(void)
{
    return cases;
}

int friend_invite_assembly_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int friend_invite_assembly_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

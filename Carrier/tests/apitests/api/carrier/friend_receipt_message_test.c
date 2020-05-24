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
    int64_t msgid;
    int state;
};

static CarrierContextExtra extra = {
    .from = NULL,
    .msg  = NULL,
    .len  = 0,
    .msgid = 0,
    .state = -1,
};

static inline void wakeup(void* context)
{
    cond_signal(((CarrierContext *)context)->cond);
}

static void ready_cb(ElaCarrier *w, void *context)
{
    cond_signal(((CarrierContext *)context)->ready_cond);
}

static void kill_node()
{
    int rc;
    char buf[2][32] = {0};

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

    rc = write_cmd("restartnode %d\n", 10000);
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s", buf[0]);
    CU_ASSERT_EQUAL_FATAL(rc, 1);
    CU_ASSERT_STRING_EQUAL_FATAL(buf[0], "ready");
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
                              int64_t timestamp, bool is_receipt, void *context)
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

static void test_request_friend_by_express(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    bool is_offline;
    int64_t msgid = 0;
    bool is_wakeup;
    char buf[2][32] = {0};
    int rc;

    test_context.context_reset(&test_context);

    rc = remove_friend_anyway(&test_context, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FALSE_FATAL(ela_is_friend(wctxt->carrier, robotid));
    usleep(2000000);

    kill_node();

    usleep(2000000);
    rc = ela_add_friend(wctxt->carrier, robotaddr, "auto-reply");
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    cond_wait(wctxt->cond);

    start_node();

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL_FATAL(buf[0], "fadd");
    CU_ASSERT_STRING_EQUAL_FATAL(buf[1], "succeeded");
    status_cond_wait(wctxt->friend_status_cond, ONLINE);
}

static void test_message_receipt_cb(int64_t msgid,  ElaReceiptState state, void *context)
{
    vlogV("========= msgid=%lld, stat=%d", msgid, state);
    CarrierContextExtra *extra = ((CarrierContext *)context)->extra;
    extra->msgid  = msgid;
    extra->state  = state;

    wakeup(context);
}

static void test_send_message_to_friend_with_receipt(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    bool is_offline;
    int64_t msgid = 0;
    bool is_wakeup;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    const char* out = "message-receipt-test";
    msgid = ela_send_message_with_receipt(wctxt->carrier, robotid, out, strlen(out),
                                          test_message_receipt_cb, wctxt);
    CU_ASSERT(msgid > 0);

    is_wakeup = cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(is_wakeup);
    if (is_wakeup) {
        CU_ASSERT(extra->msgid == msgid);
        CU_ASSERT(extra->state == ElaReceipt_ByFriend);
    }

    char in[64];
    rc = read_ack("%64s", in);
    CU_ASSERT_EQUAL(rc, 1);
    CU_ASSERT_STRING_EQUAL(in, out);
}

static void test_send_bulkmsg_to_friend_with_receipt(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    bool is_offline;
    int64_t msgid = 0;
    bool is_wakeup;
    int idx;
    const int datalen = 4096;
    char *in, *out;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    out = (char*)calloc(1, datalen);
    for(idx = 0; idx < datalen; idx++) {
        out[idx] = '0' + (idx % 8);
    }
    memcpy(out + datalen - 5, "end", 4);
    msgid = ela_send_message_with_receipt(wctxt->carrier, robotid, out, strlen(out),
                                          test_message_receipt_cb, wctxt);
    CU_ASSERT(msgid > 0);

    is_wakeup = cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(is_wakeup);
    if (is_wakeup) {
        CU_ASSERT(extra->msgid == msgid);
        CU_ASSERT(extra->state == ElaReceipt_ByFriend);
    }

    in = (char*)calloc(1, datalen);
    rc = read_ack("%5192s", in);
    CU_ASSERT_EQUAL(rc, 1);
    CU_ASSERT_STRING_EQUAL(in, out);

    free(in);
    free(out);
}

static void test_send_offmsg_to_friend_with_receipt(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char buf[2][32] = {0};
    bool is_offline;
    int64_t msgid = 0;
    bool is_wakeup;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    kill_node();

    status_cond_wait(wctxt->friend_status_cond, OFFLINE);

    const char* out = "offline-message-receipt-test";
    msgid = ela_send_message_with_receipt(wctxt->carrier, robotid, out, strlen(out),
                                          test_message_receipt_cb, wctxt);
    CU_ASSERT(msgid > 0);

    is_wakeup = cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(is_wakeup);
    if (is_wakeup) {
        CU_ASSERT(extra->msgid == msgid);
        CU_ASSERT(extra->state == ElaReceipt_Offline);
    }

    start_node();

    status_cond_wait(wctxt->friend_status_cond, ONLINE);

    char in[64];
    rc = read_ack("%64s", in);
    CU_ASSERT_EQUAL(rc, 1);
    CU_ASSERT_STRING_EQUAL(in, out);
}

static void test_send_offbmsg_to_friend_with_receipt(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char buf[2][32] = {0};
    bool is_offline;
    int64_t msgid = 0;
    bool is_wakeup;
    int idx;
    const int datalen = 4096;
    char *in, *out;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    kill_node();

    status_cond_wait(wctxt->friend_status_cond, OFFLINE);

    out = (char*)calloc(1, datalen);
    for(idx = 0; idx < datalen; idx++) {
        out[idx] = '0' + (idx % 8);
    }
    memcpy(out + datalen - 5, "end", 4);
    msgid = ela_send_message_with_receipt(wctxt->carrier, robotid, out, strlen(out),
                                          test_message_receipt_cb, wctxt);
    CU_ASSERT(msgid > 0);

    is_wakeup = cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(is_wakeup);
    if (is_wakeup) {
        CU_ASSERT(extra->msgid == msgid);
        CU_ASSERT(extra->state == ElaReceipt_Offline);
    }

    start_node();

    status_cond_wait(wctxt->friend_status_cond, ONLINE);

    in = (char*)calloc(1, datalen);
    rc = read_ack("%5192s", in);
    CU_ASSERT_EQUAL(rc, 1);
    CU_ASSERT_STRING_EQUAL(in, out);

    free(in);
    free(out);
}

static void test_send_edgemsg_to_friend_with_receipt(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char buf[2][32] = {0};
    bool is_offline;
    int64_t msgid = 0;
    bool is_wakeup;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    kill_node();

    // status_cond_wait(wctxt->friend_status_cond, OFFLINE);

    const char* out = "edge-message-receipt-test";
    msgid = ela_send_message_with_receipt(wctxt->carrier, robotid, out, strlen(out),
                                          test_message_receipt_cb, wctxt);
    CU_ASSERT(msgid > 0);

    is_wakeup = cond_trywait(wctxt->cond, 60000);
    CU_ASSERT_TRUE(is_wakeup);
    if (is_wakeup) {
        CU_ASSERT(extra->msgid == msgid);
        CU_ASSERT(extra->state == ElaReceipt_Offline);
    }

    start_node();

    status_cond_wait(wctxt->friend_status_cond, ONLINE);

    char in[64];
    rc = read_ack("%64s", in);
    CU_ASSERT_EQUAL(rc, 1);
    CU_ASSERT_STRING_EQUAL(in, out);
}

static CU_TestInfo cases[] = {
    { "test_request_friend_by_express", test_request_friend_by_express },
    { "test_send_message_to_friend_with_receipt", test_send_message_to_friend_with_receipt },
    { "test_send_bulkmsg_to_friend_with_receipt", test_send_bulkmsg_to_friend_with_receipt },
    { "test_send_offmsg_to_friend_with_receipt", test_send_offmsg_to_friend_with_receipt },
    { "test_send_offbmsg_to_friend_with_receipt", test_send_offbmsg_to_friend_with_receipt },
    { "test_send_edgemsg_to_friend_with_receipt", test_send_edgemsg_to_friend_with_receipt },
    {NULL, NULL }
};

CU_TestInfo *friend_receipt_message_test_get_cases(void)
{
    return cases;
}

int friend_receipt_message_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int friend_receipt_message_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

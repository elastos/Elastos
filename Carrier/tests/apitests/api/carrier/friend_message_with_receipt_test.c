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
    .friend_message  = friend_message_cb,
    .friend_invite   = NULL
};

static Condition DEFINE_COND(ready_cond);
static Condition DEFINE_COND(cond);
static StatusCondition DEFINE_STATUS_COND(friend_status_cond);
static Condition DEFINE_COND(receipts_cond);

static CarrierContext carrier_context = {
    .cbs = &callbacks,
    .carrier = NULL,
    .ready_cond = &ready_cond,
    .cond = &cond,
    .friend_status_cond = &friend_status_cond,
    .receipts_cond = &receipts_cond,
    .extra = &extra
};

static void test_context_reset(TestContext *context)
{
    cond_reset(context->carrier->cond);
    status_cond_reset(context->carrier->friend_status_cond);
    cond_reset(context->carrier->receipts_cond);
}

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = NULL,
    .stream  = NULL,
    .context_reset = test_context_reset
};

static void message_receipt_cb(int64_t msgid,  ElaReceiptState state,
                               void *context)
{
    CarrierContextExtra *extra = ((CarrierContext *)context)->extra;
    extra->msgid  = msgid;
    extra->state  = state;

    vlogD("message receipt callback invoked (msgid: %llx, stat: %d)" , msgid, state);
    cond_signal(((CarrierContext *)context)->receipts_cond);
}

static void test_send_message_with_receipt(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char msg[1024] = {0};
    char in[1024] = {0};
    int64_t msgid = 0;
    bool wakeup;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    memset(msg, 'm', sizeof(msg) -1);
    msgid = ela_send_message_with_receipt(wctxt->carrier, robotid, msg, sizeof(msg),
                                          message_receipt_cb, wctxt);
    CU_ASSERT_TRUE_FATAL(msgid > 0);

    wakeup = cond_trywait(wctxt->receipts_cond, 60000);
    CU_ASSERT_TRUE_FATAL(wakeup);
    CU_ASSERT_EQUAL(extra->msgid, msgid);
    CU_ASSERT_EQUAL(extra->state, ElaReceipt_ByFriend);

    rc = read_ack("%1024s", in);
    CU_ASSERT_EQUAL_FATAL(rc, 1);
    CU_ASSERT_STRING_EQUAL(in, msg);
}

static void test_send_bulkmsg_with_receipt(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    size_t bulksz = ELA_MAX_APP_BULKMSG_LEN;
    char *bulkmsg;
    int64_t msgid = 0;
    int size;
    char buf[32] = {0};
    bool wakeup;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    bulkmsg = (char *)calloc(1, bulksz);
    if (!bulkmsg) {
        vlogF("Panic::oom !!!");
        return;
    }
    memset(bulkmsg, 'b', bulksz - 1);

    msgid = ela_send_message_with_receipt(wctxt->carrier, robotid, bulkmsg, bulksz,
                                          message_receipt_cb, wctxt);
    CU_ASSERT_TRUE_FATAL(msgid > 0);

    wakeup = cond_trywait(wctxt->receipts_cond, 60000);
    CU_ASSERT_TRUE_FATAL(wakeup);
    CU_ASSERT_EQUAL(extra->msgid, msgid);
    CU_ASSERT_EQUAL(extra->state, ElaReceipt_ByFriend);

    rc = read_ack("%64s %d", buf, &size);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_EQUAL(size, bulksz);
    CU_ASSERT_STRING_EQUAL(buf, "bulkmsg");

    free(bulkmsg);
}

static void test_send_offmsg_with_receipt(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char prefix[32] = {0};
    char buf[2][32] = {0};
    char ack[32] = {0};
    char msg[32] = {0};
    int64_t msgid = 0;
    bool wakeup;
    int rc;

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
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "offmsgprefix");
    CU_ASSERT_STRING_EQUAL(buf[1], "success");


    sprintf(msg, "%s%s", prefix, "receipt");
    msgid = ela_send_message_with_receipt(wctxt->carrier, robotid, msg, strlen(msg),
                                          message_receipt_cb, wctxt);
    CU_ASSERT_TRUE_FATAL(msgid > 0);

    wakeup = cond_trywait(wctxt->receipts_cond, 60000);
    CU_ASSERT_TRUE_FATAL(wakeup);
    CU_ASSERT_EQUAL(extra->msgid, msgid);
    CU_ASSERT_EQUAL(extra->state, ElaReceipt_Offline);
    usleep(5000000);

    rc = write_cmd("restartnode %d\n", 900);
    CU_ASSERT_TRUE_FATAL(rc > 0);

    // in offmsg case, robot would not ack with "ready" to testcase.
    status_cond_wait(wctxt->friend_status_cond, ONLINE);

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "offmsg");
    CU_ASSERT_STRING_EQUAL(buf[1], msg);
}

static void test_send_offline_bulkmsg_with_receipt(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    size_t bulksz = 1*1024*1024;
    char *bulkmsg;
    int64_t msgid = 0;
    int size;
    char prefix[32] = {0};
    char buf[2][32] = {0};
    bool wakeup;
    int rc;

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
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "offmsgprefix");
    CU_ASSERT_STRING_EQUAL(buf[1], "success");

    bulkmsg = (char *)calloc(1, bulksz);
    if (!bulkmsg) {
        vlogF("Panic::oom !!!");
        return;
    }

    sprintf(bulkmsg, "%s", prefix);
    memset(bulkmsg + strlen(prefix), 'b', bulksz - strlen(prefix) -1);

    msgid = ela_send_message_with_receipt(wctxt->carrier, robotid, bulkmsg, bulksz,
                                          message_receipt_cb, wctxt);
    CU_ASSERT_TRUE_FATAL(msgid > 0);

    wakeup = cond_trywait(wctxt->receipts_cond, 60000);
    CU_ASSERT_TRUE_FATAL(wakeup);
    CU_ASSERT_EQUAL(extra->msgid, msgid);
    CU_ASSERT_EQUAL(extra->state, ElaReceipt_Offline);
    usleep(5000000);

    rc = write_cmd("restartnode %d\n", 900);
    CU_ASSERT_TRUE_FATAL(rc > 0);

    // in offmsg case, robot would not ack with "ready" to testcase.
    status_cond_wait(wctxt->friend_status_cond, ONLINE);

    rc = read_ack("%32s %d", buf[0], &size);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_EQUAL(size, bulksz);
    CU_ASSERT_STRING_EQUAL(buf[0], "bulkmsg");
}

static void test_send_msg_with_receipt_in_edge_case(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *extra = wctxt->extra;
    char prefix[32] = {0};
    char buf[2][32] = {0};
    char ack[32] = {0};
    char msg[32] = {0};
    int64_t msgid = 0;
    bool wakeup;
    int rc;

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

    sprintf(prefix, "%ld:", time(NULL));
    rc = write_cmd("offmsgprefix %s\n", prefix);
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL_FATAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "offmsgprefix");
    CU_ASSERT_STRING_EQUAL(buf[1], "success");


    sprintf(msg, "%s%s", prefix, "receipt");
    msgid = ela_send_message_with_receipt(wctxt->carrier, robotid, msg, strlen(msg),
                                          message_receipt_cb, wctxt);
    CU_ASSERT_TRUE_FATAL(msgid > 0);

    status_cond_wait(wctxt->friend_status_cond, OFFLINE);

    wakeup = cond_trywait(wctxt->receipts_cond, 60000);
    CU_ASSERT_TRUE_FATAL(wakeup);
    CU_ASSERT_EQUAL(extra->msgid, msgid);
    CU_ASSERT_EQUAL(extra->state, ElaReceipt_Offline);
    usleep(5000000);

    rc = write_cmd("restartnode %d\n", 900);
    CU_ASSERT_TRUE_FATAL(rc > 0);

    // in offmsg case, robot would not ack with "ready" to testcase.
    status_cond_wait(wctxt->friend_status_cond, ONLINE);

    rc = read_ack("%32s %32s", buf[0], buf[1]);
    CU_ASSERT_EQUAL(rc, 2);
    CU_ASSERT_STRING_EQUAL(buf[0], "offmsg");
    CU_ASSERT_STRING_EQUAL(buf[1], msg);
}

static CU_TestInfo cases[] = {
    { "test_send_message_with_receipt", test_send_message_with_receipt },
    { "test_send_bulkmsg_with_receipt", test_send_bulkmsg_with_receipt },
    { "test_send_offmsg_with_receipt",  test_send_offmsg_with_receipt  },
    //{ "test_send_offline_bulkmsg_with_receipt", test_send_offline_bulkmsg_with_receipt },
    { "test_send_msg_with_receipt_in_edge_case", test_send_msg_with_receipt_in_edge_case },
    {NULL, NULL }
};

CU_TestInfo *friend_message_with_receipt_test_get_cases(void)
{
    return cases;
}

int friend_message_with_receipt_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int friend_message_with_receipt_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);
    return 0;
}

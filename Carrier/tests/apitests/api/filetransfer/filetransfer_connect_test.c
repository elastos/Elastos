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

#include <stdlib.h>

#include <CUnit/Basic.h>
#include <crystal.h>

#include "ela_carrier.h"
#include "ela_filetransfer.h"

#include "config.h"
#include "cond.h"
#include "test_helper.h"

struct CarrierContextExtra {
    ElaFileTransferInfo ft_info;
    bool has_ft_info;
};

static CarrierContextExtra extra = {
    .ft_info = {0},
    .has_ft_info = false
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

static void ft_state_changed_cb(ElaFileTransfer *filetransfer,
                                FileTransferConnection state, void *context)
{
    TestContext *wctx = (TestContext *)context;
    CarrierContext *ctx = wctx->carrier;

    ctx->ft_con_state = state;
    ctx->ft_con_state_bits |= (1 << state);
    cond_signal(ctx->ft_cond);
}

static void ft_connect_cb(ElaCarrier *carrier,
                          const char *address,
                          const ElaFileTransferInfo *fileinfo,
                          void *context)
{
    TestContext *wctx = (TestContext *)context;
    CarrierContext *ctx = wctx->carrier;
    CarrierContextExtra *extra = ctx->extra;

    if (fileinfo) {
        extra->has_ft_info = true;
        memcpy(&extra->ft_info, fileinfo, sizeof(ElaFileTransferInfo));
    }

    cond_signal(ctx->ft_cond);
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

static ElaFileTransferCallbacks ft_cbs = {
    .state_changed = ft_state_changed_cb,
    .file = NULL,
    .pull = NULL,
    .data = NULL,
    .pending = NULL,
    .resume = NULL,
    .cancel = NULL
};

static Condition DEFINE_COND(ready_cond);
static Condition DEFINE_COND(cond);
static Condition DEFINE_COND(ft_cond);
static StatusCondition DEFINE_STATUS_COND(friend_status_cond);

static CarrierContext carrier_context = {
    .cbs = &callbacks,
    .carrier = NULL,
    .ft = NULL,
    .ft_info = NULL,
    .ft_cbs = &ft_cbs,
    .ft_cond = &ft_cond,
    .ready_cond = &ready_cond,
    .cond = &cond,
    .friend_status_cond = &friend_status_cond,
    .extra = &extra
};

static void test_context_reset(TestContext *context)
{
    cond_reset(context->carrier->cond);
    cond_reset(context->carrier->ft_cond);
    status_cond_reset(context->carrier->friend_status_cond);
    context->carrier->ft_con_state_bits = 0;
}

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = NULL,
    .stream  = NULL,
    .context_reset = test_context_reset
};

static void test_filetransfer_connect(void)
{
    test_filetransfer_scheme(&test_context, NULL, false);
}

static void test_filetransfer_connect_with_file_info(void)
{
    ElaFileTransferInfo ft_info = {0};

    strcpy(ft_info.filename, "test-file-name");
    ft_info.size = 1;
    carrier_context.ft_info = &ft_info;

    test_filetransfer_scheme(&test_context, NULL, true);
}

static void test_filetransfer_accept_connect(void)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *wextra = wctxt->extra;
    ElaFileTransferInfo *ft_info;
    char userid[ELA_MAX_ID_LEN + 1] = {0};
    char cmd[32] = {0};
    char result[32] = {0};
    uint8_t ft_con_state_bits = 0;
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    rc = ela_filetransfer_init(wctxt->carrier, ft_connect_cb, &test_context);
    CU_ASSERT_EQUAL(rc, 0);

    rc = write_cmd("ft_init\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_init") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    rc = write_cmd("ft_new %s\n", userid);
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_new") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    rc = write_cmd("ft_connect\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_connect") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // Wait for the ft_connect_cb to be invoked.
    cond_wait(wctxt->ft_cond);

    ft_info = wextra->has_ft_info ? &wextra->ft_info : NULL;
    wctxt->ft = ela_filetransfer_new(wctxt->carrier, robotid, ft_info,
                                     wctxt->ft_cbs, &test_context);
    CU_ASSERT_PTR_NOT_NULL_FATAL(wctxt->ft);

    rc = ela_filetransfer_accept_connect(wctxt->ft);
    CU_ASSERT_EQUAL(rc, 0);

    /* Wait for the ft_state_changed_cb to be invoked. After its invocation,
       the file transfer connection state should have changed to be
       FileTransferConnection_connecting. */
    cond_wait(wctxt->ft_cond);

    /* Wait for the ft_state_changed_cb to be invoked. After its invocation,
       the file transfer connection state should have changed to be
       FileTransferConnection_connected. */
    cond_wait(wctxt->ft_cond);

    ft_con_state_bits |= 1 << FileTransferConnection_connecting;
    ft_con_state_bits |= 1 << FileTransferConnection_connected;
    CU_ASSERT_EQUAL_FATAL(wctxt->ft_con_state_bits, ft_con_state_bits);

    rc = write_cmd("ft_cleanup\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_cleanup") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    /* Wait for the ft_state_changed_cb to be invoked. After invocation,
       the filetransfer connection state should have changed to be
       FileTransferConnection_closed. */
    cond_wait(wctxt->ft_cond);

    ft_con_state_bits |= 1 << FileTransferConnection_closed;
    CU_ASSERT_EQUAL(wctxt->ft_con_state_bits, ft_con_state_bits);

    ela_filetransfer_close(wctxt->ft);
    ela_filetransfer_cleanup(wctxt->carrier);
}

static CU_TestInfo cases[] = {
    { "test_filetransfer_connect", test_filetransfer_connect },
    { "test_filetransfer_connect_with_file_info", test_filetransfer_connect_with_file_info },
    { "test_filetransfer_accept_connect", test_filetransfer_accept_connect },
    { NULL, NULL }
};

CU_TestInfo *filetransfer_connect_test_get_cases(void)
{
    return cases;
}

int filetransfer_connect_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0)
        CU_FAIL("Error: test suite initialize error");

    return rc;
}

int filetransfer_connect_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);
    return 0;
}

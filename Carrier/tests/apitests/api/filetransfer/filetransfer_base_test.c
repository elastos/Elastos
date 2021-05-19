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
    int status;
    char reason[255];
};

static CarrierContextExtra extra = {
    .status = 0,
    .reason = {0}
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

static void ft_pull_cb(ElaFileTransfer *filetransfer, const char *fileid,
                       uint64_t offset, void *context)
{
    TestContext *wctx = (TestContext *)context;
    CarrierContext *ctx = wctx->carrier;

    cond_signal(ctx->ft_cond);
}

static void ft_pending_cb(ElaFileTransfer *filetransfer, const char *fileid,
                          void *context)
{
    TestContext *wctx = (TestContext *)context;
    CarrierContext *ctx = wctx->carrier;

    cond_signal(ctx->ft_cond);
}

static void ft_resume_cb(ElaFileTransfer *filetransfer, const char *fileid,
                         void *context)
{
    TestContext *wctx = (TestContext *)context;
    CarrierContext *ctx = wctx->carrier;

    cond_signal(ctx->ft_cond);
}

static void ft_cancel_cb(ElaFileTransfer *filetransfer, const char *fileid,
                         int status, const char *reason, void *context)
{
    TestContext *wctx = (TestContext *)context;
    CarrierContext *ctx = wctx->carrier;
    CarrierContextExtra *extra = ctx->extra;

    extra->status = status;
    if (reason)
        strcpy(extra->reason, reason);

    cond_signal(ctx->ft_cond);
}

static ElaFileTransferCallbacks ft_cbs = {
    .state_changed = ft_state_changed_cb,
    .file = NULL,
    .pull = ft_pull_cb,
    .data = NULL,
    .pending = ft_pending_cb,
    .resume = ft_resume_cb,
    .cancel = ft_cancel_cb
};

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

static int filetransfer_base_cb(TestContext *ctx)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *wextra = wctxt->extra;
    ElaFileTransferInfo ft_info = {0};
    char file_name[ELA_MAX_FILE_NAME_LEN + 1] = {0};
    char file_id[ELA_MAX_FILE_ID_LEN + 1] = {0};
    char cmd[32] = {0};
    char result[32] = {0};
    char data[ELA_MAX_USER_DATA_LEN] = "hello";
    char *p;
    const char *reason = "busy";
    int len = (int)strlen(data) + 1;
    int status = -1;
    int rc;
    int size;

    p = ela_filetransfer_fileid(ft_info.filename, sizeof(ft_info.filename));
    CU_ASSERT_PTR_EQUAL(p, ft_info.filename);
    ft_info.size = 10000;

    rc = ela_filetransfer_add(wctxt->ft, &ft_info);
    CU_ASSERT_EQUAL(rc, 0);

    rc = read_ack("%64s %64s %d", file_name, ft_info.fileid, &size);
    CU_ASSERT_TRUE(rc == 3);
    CU_ASSERT_TRUE(strcmp(file_name, ft_info.filename) == 0);
    CU_ASSERT_TRUE(size == 10000);

    rc = write_cmd("ft_pull\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_pull") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait for the 'pull' callback function to be invoked
    cond_wait(wctxt->ft_cond);

    rc = (int)ela_filetransfer_send(wctxt->ft, ft_info.fileid, (const uint8_t*)data, len);
    CU_ASSERT_EQUAL(rc, len);

    rc = read_ack("%32s %45s %32s %d", cmd, file_id, data, &len);
    CU_ASSERT_TRUE(rc == 4);
    CU_ASSERT_TRUE(strcmp(cmd, "ft_data") == 0);
    CU_ASSERT_TRUE(strcmp(file_id, ft_info.fileid) == 0);
    CU_ASSERT_TRUE(strcmp(data, "hello") == 0);
    CU_ASSERT_TRUE(len == strlen("hello") + 1);

    rc = write_cmd("ft_pend\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_pend") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait for the 'pending' callback function to be invoked
    cond_wait(wctxt->ft_cond);

    rc = write_cmd("ft_resume\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_resume") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait for the 'resume' callback function to be invoked
    cond_wait(wctxt->ft_cond);

    memset(data, 'D', sizeof(data) - 1);
    data[sizeof(data) - 1] = '\0';
    rc = (int)ela_filetransfer_send(wctxt->ft, ft_info.fileid, (const uint8_t*)data, sizeof(data) - 1);
    CU_ASSERT_EQUAL(rc, sizeof(data) - 1);

    rc = read_ack("%32s %45s %32s %d", cmd, file_id, data, &len);
    CU_ASSERT_TRUE(rc == 4);
    CU_ASSERT_TRUE(strcmp(cmd, "ft_data") == 0);
    CU_ASSERT_TRUE(strcmp(file_id, ft_info.fileid) == 0);
    CU_ASSERT_TRUE(strcmp(data, "bigdata") == 0);
    CU_ASSERT_TRUE(len == sizeof(data) - 1);

    rc = write_cmd("ft_cancel %d %s\n", status, reason);
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_cancel") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait for the 'cancel' callback function to be invoked
    cond_wait(wctxt->ft_cond);
    CU_ASSERT_TRUE(wextra->status == status);
    CU_ASSERT_TRUE(strcmp(wextra->reason, reason) == 0);

    return 0;
}

static int filetransfer_with_zero_length_cb(TestContext *ctx)
{
    CarrierContext *wctxt = test_context.carrier;
    CarrierContextExtra *wextra = wctxt->extra;
    ElaFileTransferInfo ft_info = {0};
    char file_name[ELA_MAX_FILE_NAME_LEN + 1] = {0};
    char file_id[ELA_MAX_FILE_ID_LEN + 1] = {0};
    char cmd[32] = {0};
    char result[32] = {0};
    char data[32] = {0};
    char *p;
    int len = (int)strlen(data) + 1;
    int rc;
    int size;

    p = ela_filetransfer_fileid(ft_info.filename, sizeof(ft_info.filename));
    CU_ASSERT_PTR_EQUAL(p, ft_info.filename);
    ft_info.size = 10000;

    rc = ela_filetransfer_add(wctxt->ft, &ft_info);
    CU_ASSERT_EQUAL(rc, 0);

    rc = read_ack("%64s %64s %d", file_name, ft_info.fileid, &size);
    CU_ASSERT_TRUE(rc == 3);
    CU_ASSERT_TRUE(strcmp(file_name, ft_info.filename) == 0);
    CU_ASSERT_TRUE(size == 10000);

    rc = write_cmd("ft_pull\n");
    CU_ASSERT_FATAL(rc > 0);

    rc = read_ack("%32s %32s", cmd, result);
    CU_ASSERT_TRUE_FATAL(rc == 2);
    CU_ASSERT_TRUE_FATAL(strcmp(cmd, "ft_pull") == 0);
    CU_ASSERT_TRUE_FATAL(strcmp(result, "succeeded") == 0);

    // wait for the 'pull' callback function to be invoked
    cond_wait(wctxt->ft_cond);

    rc = (int)ela_filetransfer_send(wctxt->ft, ft_info.fileid, NULL, 0);
    CU_ASSERT_EQUAL(rc, 0);

    rc = read_ack("%32s %45s %32s %d", cmd, file_id, data, &len);
    CU_ASSERT_TRUE(rc == 4);
    CU_ASSERT_TRUE(strcmp(cmd, "ft_data") == 0);
    CU_ASSERT_TRUE(strcmp(file_id, ft_info.fileid) == 0);
    CU_ASSERT_TRUE(strcmp(data, "null") == 0);
    CU_ASSERT_TRUE(len == 0);

    return 0;
}

static void test_filetransfer_base(void)
{
    test_filetransfer_scheme(&test_context, filetransfer_base_cb, false);
}

static void test_filetransfer_with_zero_length(void)
{
    test_filetransfer_scheme(&test_context, filetransfer_with_zero_length_cb, false);
}

static CU_TestInfo cases[] = {
    { "test_filetransfer_base", test_filetransfer_base },
    { "test_filetransfer_with_zero_length", test_filetransfer_with_zero_length },
    { NULL, NULL }
};

CU_TestInfo *filetransfer_base_test_get_cases(void)
{
    return cases;
}

int filetransfer_base_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0)
        CU_FAIL("Error: test suite initialize error");

    return rc;
}

int filetransfer_base_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);
    return 0;
}

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

#define FILE_COUNT 5

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

static ElaFileTransferCallbacks ft_cbs = {
    .state_changed = ft_state_changed_cb,
    .file = NULL,
    .pull = NULL,
    .data = NULL,
    .pending = NULL,
    .resume = NULL,
    .cancel = NULL
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
    .extra = NULL
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

static int filetransfer_id_name_cb(TestContext *ctx)
{
    CarrierContext *wctxt = test_context.carrier;
    ElaFileTransferInfo ft_info[FILE_COUNT] = {0};
    char file_name[ELA_MAX_FILE_NAME_LEN + 1] = {0};
    char file_id[ELA_MAX_FILE_ID_LEN + 1] = {0};
    char *user_data[FILE_COUNT] = {"data1", "data2", "data3", "data4", "data5"};
    char *p;
    int rc;
    int size;
    int i;

    for (i = 0; i < FILE_COUNT; i++) {
        p = ela_filetransfer_fileid(ft_info[i].filename, sizeof(ft_info[i].filename));
        CU_ASSERT_PTR_EQUAL(p, ft_info[i].filename);

        ft_info[i].size = i + 1;
    }

    for (i = 0; i < FILE_COUNT; i++) {
        rc = ela_filetransfer_add(wctxt->ft, &ft_info[i]);
        CU_ASSERT_EQUAL(rc, 0);

        rc = read_ack("%64s %64s %d", file_name, ft_info[i].fileid, &size);
        CU_ASSERT_TRUE(rc == 3);

        CU_ASSERT_TRUE(strcmp(file_name, ft_info[i].filename) == 0);

        CU_ASSERT_TRUE(size == i + 1);

        rc = ela_filetransfer_set_userdata(wctxt->ft, ft_info[i].fileid, user_data[i]);
        CU_ASSERT_EQUAL(rc, 0);
    }

    for (i = 0; i < FILE_COUNT; i++) {
        p = ela_filetransfer_get_fileid(wctxt->ft, ft_info[i].filename,
                                        file_id, sizeof(file_id));
        CU_ASSERT_PTR_EQUAL(p, file_id);
        CU_ASSERT_TRUE(strcmp(file_id, ft_info[i].fileid) == 0);

        p = ela_filetransfer_get_filename(wctxt->ft, ft_info[i].fileid,
                                          file_name, sizeof(file_name));
        CU_ASSERT_PTR_EQUAL(p, file_name);
        CU_ASSERT_TRUE(strcmp(file_name, ft_info[i].filename) == 0);

        p = ela_filetransfer_get_userdata(wctxt->ft, ft_info[i].fileid);
        CU_ASSERT_TRUE_FATAL(strcmp(p, user_data[i]) == 0);
    }

    return 0;
}

static void test_filetransfer_id_name(void)
{
    test_filetransfer_scheme(&test_context, filetransfer_id_name_cb, false);
}

static CU_TestInfo cases[] = {
    { "test_filetransfer_id_name", test_filetransfer_id_name },
    { NULL, NULL }
};

CU_TestInfo *filetransfer_id_name_test_get_cases(void)
{
    return cases;
}

int filetransfer_id_name_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0)
        CU_FAIL("Error: test suite initialize error");

    return rc;
}

int filetransfer_id_name_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);
    return 0;
}

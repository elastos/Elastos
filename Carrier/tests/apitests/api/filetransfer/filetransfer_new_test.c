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
    .ft = NULL,
    .ft_info = NULL,
    .ft_cbs = NULL,
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

static void test_filetransfer_init(void)
{
    CarrierContext *wctxt = test_context.carrier;
    int rc;

    rc = ela_filetransfer_init(wctxt->carrier, NULL, NULL);
    CU_ASSERT_EQUAL(rc, 0);

    ela_filetransfer_cleanup(wctxt->carrier);
}

static void test_filetransfer_new(void)
{
    CarrierContext *wctxt = test_context.carrier;
    ElaFileTransferInfo ft_info = {0};
    ElaFileTransferCallbacks ft_cbs = {0};
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    rc = ela_filetransfer_init(wctxt->carrier, NULL, NULL);
    CU_ASSERT_EQUAL(rc, 0);

    wctxt->ft_cbs = &ft_cbs;
    wctxt->ft = ela_filetransfer_new(wctxt->carrier, robotid, NULL,
                                     wctxt->ft_cbs, NULL);
    CU_ASSERT_PTR_NOT_NULL(wctxt->ft);
    ela_filetransfer_close(wctxt->ft);

    strcpy(ft_info.filename, "test-file-name");
    ft_info.size = 1;
    wctxt->ft_info = &ft_info;
    wctxt->ft = ela_filetransfer_new(wctxt->carrier, robotid, wctxt->ft_info,
                                     wctxt->ft_cbs, NULL);
    CU_ASSERT_PTR_NOT_NULL(wctxt->ft);

    ela_filetransfer_close(wctxt->ft);
    ela_filetransfer_cleanup(wctxt->carrier);
}

static void test_filetransfer_new_with_stranger(void)
{
    CarrierContext *wctxt = test_context.carrier;
    ElaFileTransferInfo ft_info = {0};
    ElaFileTransferCallbacks ft_cbs = {0};
    int rc;

    test_context.context_reset(&test_context);

    rc = remove_friend_anyway(&test_context, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_FALSE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    rc = ela_filetransfer_init(wctxt->carrier, NULL, NULL);
    CU_ASSERT_EQUAL(rc, 0);

    strcpy(ft_info.filename, "test-file-name");
    ft_info.size = 1;
    wctxt->ft_info = &ft_info;
    wctxt->ft_cbs = &ft_cbs;
    wctxt->ft = ela_filetransfer_new(wctxt->carrier, robotid, wctxt->ft_info,
                                     wctxt->ft_cbs, NULL);
    CU_ASSERT_EQUAL(wctxt->ft, NULL);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));

    ela_filetransfer_cleanup(wctxt->carrier);
}

static void test_filetransfer_new_without_initializing(void)
{
    CarrierContext *wctxt = test_context.carrier;
    ElaFileTransferInfo ft_info = {0};
    ElaFileTransferCallbacks ft_cbs = {0};
    int rc;

    test_context.context_reset(&test_context);

    rc = add_friend_anyway(&test_context, robotid, robotaddr);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(ela_is_friend(wctxt->carrier, robotid));

    strcpy(ft_info.filename, "test-file-name");
    ft_info.size = 1;
    wctxt->ft_info = &ft_info;
    wctxt->ft_cbs = &ft_cbs;
    wctxt->ft = ela_filetransfer_new(wctxt->carrier, robotid, wctxt->ft_info,
                                     wctxt->ft_cbs, NULL);
    CU_ASSERT_EQUAL(wctxt->ft, NULL);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
}

static CU_TestInfo cases[] = {
    { "test_filetransfer_init",                     test_filetransfer_init },
    { "test_filetransfer_new",                      test_filetransfer_new },
    { "test_filetransfer_new_with_stranger",        test_filetransfer_new_with_stranger },
    { "test_filetransfer_new_without_initializing", test_filetransfer_new_without_initializing },
    { NULL, NULL }
};

CU_TestInfo *filetransfer_new_test_get_cases(void)
{
    return cases;
}

int filetransfer_new_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0)
        CU_FAIL("Error: test suite initialize error");

    return rc;
}

int filetransfer_new_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);
    return 0;
}

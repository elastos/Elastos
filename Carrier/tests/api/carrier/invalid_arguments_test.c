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
#include <vlog.h>
#if defined(_WIN32) || defined(_WIN64)
#include <posix_helper.h>
#endif

#include "ela_carrier.h"

#include "cond.h"
#include "test_helper.h"

static void ready_cb(ElaCarrier *w, void *context)
{
    cond_signal(((CarrierContext *)context)->ready_cond);
}

static bool peer_iterate_cb(const ElaGroupPeer *peer, void *context)
{
    return true;
}

static bool group_iterate_cb(const char *groupid, void *context)
{
    return true;
}

static ElaCallbacks callbacks = {
        .idle            = NULL,
        .connection_status = NULL,
        .ready           = ready_cb,
        .self_info       = NULL,
        .friend_list     = NULL,
        .friend_connection = NULL,
        .friend_info     = NULL,
        .friend_presence = NULL,
        .friend_request  = NULL,
        .friend_added    = NULL,
        .friend_removed  = NULL,
        .friend_message  = NULL,
        .friend_invite   = NULL,
        .group_invite    = NULL,
        .group_callbacks = {0}
};

static Condition DEFINE_COND(ready_cond);
static Condition DEFINE_COND(cond);

static CarrierContext carrier_context = {
        .cbs = &callbacks,
        .carrier = NULL,
        .ready_cond = &ready_cond,
        .cond = &cond,
        .friend_status_cond = NULL,
        .extra = NULL
};

static TestContext test_context = {
        .carrier = &carrier_context,
        .session = NULL,
        .stream  = NULL,
        .context_reset = NULL
};

static void test_new_group_with_invalid_arguments(void)
{
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    CarrierContext *wctx = test_context.carrier;
    int rc;

    rc = ela_new_group(NULL, groupid, sizeof(groupid));
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_new_group(wctx->carrier, NULL, sizeof(groupid));
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_new_group(wctx->carrier, groupid, sizeof(groupid) - 1);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static void test_leave_group_with_invalid_arguments(void)
{
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    CarrierContext *wctx = test_context.carrier;
    int rc;

    rc = ela_leave_group(NULL, groupid);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_leave_group(wctx->carrier, NULL);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_leave_group(wctx->carrier, "");
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static void test_group_invite_with_invalid_arguments(void)
{
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    CarrierContext *wctx = test_context.carrier;
    int rc;

    rc = ela_group_invite(NULL, groupid, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_invite(wctx->carrier, NULL, robotid);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_invite(wctx->carrier, "", robotid);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_invite(wctx->carrier, groupid, NULL);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_invite(wctx->carrier, groupid, "");
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static void test_group_join_with_invalid_arguments(void)
{
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    CarrierContext *wctx = test_context.carrier;
    const void *cookie = "cookie";
    size_t cookie_len = strlen(cookie);
    size_t length = sizeof(groupid);
    int rc;

    rc = ela_group_join(NULL, robotid, cookie, cookie_len, groupid, length);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_join(wctx->carrier, NULL, cookie, cookie_len, groupid, length);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_join(wctx->carrier, "", cookie, cookie_len, groupid, length);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_join(wctx->carrier, robotid, NULL, cookie_len, groupid, length);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_join(wctx->carrier, robotid, cookie, 0, groupid, length);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_join(wctx->carrier, robotid, cookie, cookie_len, NULL, length);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_join(wctx->carrier, robotid, cookie, cookie_len, groupid, 0);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static void test_group_message_with_invalid_arguments(void)
{
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    char msg[ELA_MAX_APP_MESSAGE_LEN + 1] = {0};
    CarrierContext *wctx = test_context.carrier;
    int rc;

    rc = ela_group_send_message(NULL, groupid, "hello", strlen("hello"));
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_send_message(wctx->carrier, NULL, "hello", strlen("hello"));
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_send_message(wctx->carrier, "", "hello", strlen("hello"));
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_send_message(wctx->carrier, groupid, NULL, strlen("hello"));
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_send_message(wctx->carrier, groupid, "hello", 0);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    memset(msg, 'M', sizeof(msg) - 1);
    rc = ela_group_send_message(wctx->carrier, groupid, msg, sizeof(msg));
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static void test_group_set_title_with_invalid_arguments(void)
{
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    char title[32] = {0};
    char overlong_title[ELA_MAX_GROUP_TITLE_LEN + 2] = {0};
    CarrierContext *wctx = test_context.carrier;
    int rc;

    rc = ela_group_set_title(NULL, groupid, title);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_set_title(wctx->carrier, NULL, title);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_set_title(wctx->carrier, "", title);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_set_title(wctx->carrier, groupid, NULL);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_set_title(wctx->carrier, groupid, "");
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    memset(overlong_title, 'O', sizeof(overlong_title) - 1);
    rc = ela_group_set_title(wctx->carrier, groupid, overlong_title);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static void test_group_get_title_with_invalid_arguments(void)
{
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    char title[ELA_MAX_GROUP_TITLE_LEN + 1] = {0}; 
    CarrierContext *wctx = test_context.carrier;
    int rc;

    rc = ela_group_get_title(NULL, groupid, title, sizeof(title));
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_title(wctx->carrier, NULL, title, sizeof(title));
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_title(wctx->carrier, "", title, sizeof(title));
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_title(wctx->carrier, groupid, NULL, sizeof(title));
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_title(wctx->carrier, groupid, title, 0);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static void test_group_get_peer_with_invalid_arguments(void)
{
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    ElaGroupPeer group_peer = {0};
    CarrierContext *wctx = test_context.carrier;
    int rc;

    rc = ela_group_get_peer(NULL, groupid, robotid, &group_peer);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_peer(wctx->carrier, NULL, robotid, &group_peer);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_peer(wctx->carrier, "", robotid, &group_peer);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_peer(wctx->carrier, groupid, NULL, &group_peer);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_peer(wctx->carrier, groupid, "", &group_peer);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_peer(wctx->carrier, groupid, robotid, NULL);
    CU_ASSERT_EQUAL_FATAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static void test_group_get_peers_with_invalid_arguments(void)
{
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    ElaGroupPeer group_peer = {0};
    CarrierContext *wctx = test_context.carrier;
    int rc;

    rc = ela_group_get_peers(NULL, groupid, peer_iterate_cb, &group_peer);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_peers(wctx->carrier, NULL, peer_iterate_cb, &group_peer);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_peers(wctx->carrier, "", peer_iterate_cb, &group_peer);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_group_get_peers(wctx->carrier, groupid, NULL, &group_peer);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static void test_group_get_groups_with_invalid_arguments(void)
{
    CarrierContext *wctx = test_context.carrier;
    int rc;

    rc = ela_get_groups(NULL, group_iterate_cb, NULL);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));

    rc = ela_get_groups(wctx->carrier, NULL, NULL);
    CU_ASSERT_EQUAL(rc, -1);
    CU_ASSERT_EQUAL(ela_get_error(), ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
}

static CU_TestInfo cases[] = {
    { "test_new_group_with_invalid_arguments",        test_new_group_with_invalid_arguments },
    { "test_leave_group_with_invalid_arguments",      test_leave_group_with_invalid_arguments },
    { "test_group_invite_with_invalid_arguments",     test_group_invite_with_invalid_arguments },
    { "test_group_join_with_invalid_arguments",       test_group_join_with_invalid_arguments },
    { "test_group_message_with_invalid_arguments",    test_group_message_with_invalid_arguments },
    { "test_group_set_title_with_invalid_arguments",  test_group_set_title_with_invalid_arguments },
    { "test_group_get_title_with_invalid_arguments",  test_group_get_title_with_invalid_arguments },
    { "test_group_get_peer_with_invalid_arguments",   test_group_get_peer_with_invalid_arguments },
    { "test_group_get_peers_with_invalid_arguments",  test_group_get_peers_with_invalid_arguments },
    { "test_group_get_groups_with_invalid_arguments", test_group_get_groups_with_invalid_arguments },
    { NULL, NULL }
};

CU_TestInfo *invalid_arguments_test_get_cases(void)
{
    return cases;
}

int invalid_arguments_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int invalid_arguments_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

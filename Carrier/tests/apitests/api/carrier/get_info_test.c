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

#include "test_helper.h"

static void ready_cb(ElaCarrier *w, void *context)
{
    cond_signal(((CarrierContext *)context)->ready_cond);
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
    .friend_invite   = NULL
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
    .stream  = NULL
};

static void test_get_self_info(void)
{
    ElaCarrier *w = test_context.carrier->carrier;
    int rc;
    char *p;
    ElaUserInfo me;
    ElaUserInfo info = {
        .name   = {"zhangsan"},
        .description = { "We all want a code to live by." },
        .gender = { "male" },
        .phone  = { "01012345" },
        .email  = { "zhangsan@163.com" },
        .region = { "Beijing" },
        .has_avatar = 0
    };

    memset(&me, 0, sizeof(me));

    p = ela_get_userid(w, info.userid, sizeof(info.userid));
    CU_ASSERT_PTR_NOT_NULL_FATAL(p);

    rc = ela_set_self_info(w, &info);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    rc = ela_get_self_info(w, &me);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    CU_ASSERT_STRING_EQUAL(me.userid, info.userid);
    CU_ASSERT_STRING_EQUAL(me.region, info.region);
    CU_ASSERT_STRING_EQUAL(me.phone, info.phone);
    CU_ASSERT_STRING_EQUAL(me.name, info.name);
    CU_ASSERT_STRING_EQUAL(me.gender, info.gender);
    CU_ASSERT_STRING_EQUAL(me.email, info.email);
    CU_ASSERT_STRING_EQUAL(me.description, info.description);
    CU_ASSERT_EQUAL(me.has_avatar, info.has_avatar);
}

static CU_TestInfo cases[] = {
    { "test_get_self_info", test_get_self_info },
    { NULL, NULL }
};

CU_TestInfo *get_info_test_get_cases(void)
{
    return cases;
}

int get_info_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0)
        CU_FAIL("Error: test suite initialize error");

    return rc;
}

int get_info_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);
    return 0;
}

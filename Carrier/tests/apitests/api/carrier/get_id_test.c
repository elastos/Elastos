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

#include "config.h"
#include "cond.h"
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

static void test_get_address(void)
{
    CarrierContext *wctxt = test_context.carrier;
    char addr[ELA_MAX_ADDRESS_LEN + 1];
    char *p;

    p = ela_get_address(wctxt->carrier, addr, sizeof(addr));
    CU_ASSERT_PTR_NOT_NULL(p);
    CU_ASSERT_TRUE(ela_address_is_valid(addr));
}

static void test_get_nodeid(void)
{
    CarrierContext *wctxt = test_context.carrier;
    char nodeid[ELA_MAX_ID_LEN + 1];
    char *p;

    p = ela_get_nodeid(wctxt->carrier, nodeid, sizeof(nodeid));
    CU_ASSERT_PTR_NOT_NULL(p);
    CU_ASSERT_PTR_EQUAL(p, nodeid);
    CU_ASSERT_TRUE(ela_id_is_valid(nodeid));
}

static void test_get_userid(void)
{
    CarrierContext *wctxt = test_context.carrier;
    char userid[ELA_MAX_ID_LEN + 1];
    char *p;

    p = ela_get_userid(wctxt->carrier, userid, sizeof(userid));
    CU_ASSERT_PTR_NOT_NULL(p);
    CU_ASSERT_PTR_EQUAL(p, userid);
    CU_ASSERT_TRUE(ela_id_is_valid(userid));
}

static void test_get_presence(void)
{
    CarrierContext *wctxt = test_context.carrier;
    ElaPresenceStatus presence;
    int rc;

    rc = ela_set_self_presence(wctxt->carrier, ElaPresenceStatus_Busy);
    CU_ASSERT_EQUAL(rc, 0);
    rc = ela_get_self_presence(wctxt->carrier, &presence);
    CU_ASSERT_EQUAL(rc, 0);
    CU_ASSERT_EQUAL(presence, ElaPresenceStatus_Busy);
}

static CU_TestInfo cases[] = {
    { "test_get_address",test_get_address },
    { "test_get_nodeid", test_get_nodeid },
    { "test_get_userid", test_get_userid },
    { "test_get_presence", test_get_presence },
    { NULL, NULL }
};

CU_TestInfo *get_id_test_get_cases(void)
{
    return cases;
}

int get_id_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0)
        CU_FAIL("Error: test suite initialize error");

    return rc;
}

int get_id_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);
    return 0;
}

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
#include <crystal.h>
#include <CUnit/Basic.h>

#include "ela_carrier.h"
#include "cond.h"
#include "test_helper.h"

struct CarrierContextExtra {
    char *groupids[32];
    uint32_t groupids_mask;
    int group_count;
    int group_total;
};

static CarrierContextExtra extra = {
    .groupids_mask  = 0,
    .group_count = 0,
    .group_total = 32
};

extern int ela_leave_all_groups(ElaCarrier *);

static void ready_cb(ElaCarrier *w, void *context)
{
    cond_signal(((CarrierContext *)context)->ready_cond);
}

static bool group_iterate_cb(const char *groupid, void *context)
{
    CarrierContextExtra *extra = (CarrierContextExtra *)context;
    assert(extra);

    if (groupid) {
        int i;

        for (i = 0; i < extra->group_total; i++) {
            if (strcmp(groupid, extra->groupids[i]) == 0) {
                extra->groupids_mask |= 1 << i;
                break;
            }
        }
        extra->group_count++;
    }

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
    .extra = &extra
};

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = NULL,
    .stream  = NULL,
    .context_reset = NULL
};

static void test_group_get_groups(void)
{
    CarrierContext *wctx = test_context.carrier;
    CarrierContextExtra *extra = wctx->extra;
    char groupid[ELA_MAX_ID_LEN + 1] = {0};
    int rc;
    int i;

    rc = ela_leave_all_groups(wctx->carrier);
    CU_ASSERT_EQUAL_FATAL(rc, 0);

    for (i = 0; i < extra->group_total; i++) {
        extra->groupids[i] = NULL;

        rc = ela_new_group(wctx->carrier, groupid, sizeof(groupid));
        CU_ASSERT_EQUAL_FATAL(rc, 0);
        CU_ASSERT_FATAL(strlen(groupid) > 0);

        extra->groupids[i] = strdup(groupid);
    }

    rc = ela_get_groups(wctx->carrier, group_iterate_cb, extra);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_EQUAL_FATAL(extra->group_count, extra->group_total);
    CU_ASSERT_EQUAL_FATAL(extra->groupids_mask, 0xFFFFFFFF);

    for (i = 0; i < extra->group_total; i++) {
        rc = ela_leave_group(wctx->carrier, extra->groupids[i]);
        CU_ASSERT_EQUAL_FATAL(rc, 0);
        free(extra->groupids[i]);
    }
}

static CU_TestInfo cases[] = {
    { "test_group_get_groups",   test_group_get_groups },
    { NULL, NULL }
};

CU_TestInfo *group_list_test_get_cases(void)
{
    return cases;
}

int group_list_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int group_list_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

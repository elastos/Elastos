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
    ElaGroupPeer group_peers[2];
    int peer_count;
};

static CarrierContextExtra extra = {
    .group_peers = {0},
    .peer_count = 0
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

static void peer_list_changed_cb(ElaCarrier *carrier, const char *groupid,
                                 void *context)
{
    CarrierContext *wctx = (CarrierContext *)context;
    assert(wctx);

    wctx->peer_list_cnt++;
    strcpy(wctx->joined_groupid, groupid);

    cond_signal(wctx->group_cond);
}

static bool peer_iterate_cb(const ElaGroupPeer *peer, void *context)
{
    CarrierContextExtra *extra = ((CarrierContext *)context)->extra;
    assert(extra);

    if (peer) {
        memcpy(&extra->group_peers[extra->peer_count], peer, sizeof(ElaGroupPeer));
        extra->peer_count++;
    }

    return true;
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
    .friend_invite   = NULL,
    .group_invite    = NULL,
    .group_callbacks = {
        .group_connected = NULL,
        .group_message = NULL,
        .group_title = NULL,
        .peer_name = NULL,
        .peer_list_changed = peer_list_changed_cb
    }
};

static Condition DEFINE_COND(ready_cond);
static Condition DEFINE_COND(cond);
static Condition DEFINE_COND(group_cond);
static StatusCondition DEFINE_STATUS_COND(friend_status_cond);

static CarrierContext carrier_context = {
    .cbs = &callbacks,
    .carrier = NULL,
    .ready_cond = &ready_cond,
    .cond = &cond,
    .friend_status_cond = &friend_status_cond,
    .group_cond = &group_cond,
    .peer_list_cnt = 0,
    .extra = &extra
};

static void test_context_reset(TestContext *context)
{
    context->carrier->peer_list_cnt = 0;
    cond_reset(context->carrier->cond);
    cond_reset(context->carrier->group_cond);
    status_cond_reset(context->carrier->friend_status_cond);
}

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = NULL,
    .stream  = NULL,
    .context_reset = test_context_reset
};

static int group_get_peer_cb(TestContext *ctx)
{
    CarrierContext *wctx = test_context.carrier;
    char userid[ELA_MAX_ID_LEN + 1] = {0};
    ElaGroupPeer peer = {0};
    int rc;

    rc = ela_group_get_peer(wctx->carrier, wctx->groupid, robotid, &peer);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(strcmp(robotid, peer.userid) == 0);

    ela_get_userid(wctx->carrier, userid, sizeof(userid));
    rc = ela_group_get_peer(wctx->carrier, wctx->groupid, userid, &peer);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_TRUE_FATAL(strcmp(userid, peer.userid) == 0);

    return 0;
}

static int group_get_peers_cb(TestContext *ctx)
{
    CarrierContext *wctx = test_context.carrier;
    CarrierContextExtra *extra = wctx->extra;
    char userid[ELA_MAX_ID_LEN + 1] = {0};
    char *p;
    int rc;
    int i;
    int weight;

    rc = ela_group_get_peers(wctx->carrier, wctx->groupid, peer_iterate_cb, wctx);
    CU_ASSERT_EQUAL_FATAL(rc, 0);
    CU_ASSERT_EQUAL_FATAL(extra->peer_count, 2);

    p = ela_get_userid(wctx->carrier, userid, sizeof(userid));
    CU_ASSERT_EQUAL_FATAL(p, userid);

    weight = 0;
    for (i = 0; i < 2; i++) {
        if (strcmp(userid, extra->group_peers[i].userid) == 0)
            weight++;

        if (strcmp(robotid, extra->group_peers[i].userid) == 0)
            weight++;
    }
    CU_ASSERT_EQUAL_FATAL(weight, 2);

    return 0;
}

static void test_group_get_peer(void)
{
    test_group_scheme(&test_context, group_get_peer_cb);
}

static void test_group_get_peers(void)
{
    test_group_scheme(&test_context, group_get_peers_cb);
}

static CU_TestInfo cases[] = {
    { "test_group_get_peer",    test_group_get_peer },
    { "test_group_get_peers",   test_group_get_peers },
    { NULL, NULL }
};

CU_TestInfo *group_peer_test_get_cases(void)
{
    return cases;
}

int group_peer_test_suite_init(void)
{
    int rc;

    rc = test_suite_init(&test_context);
    if (rc < 0) {
        CU_FAIL("Error: test suite initialize error");
        return -1;
    }

    return 0;
}

int group_peer_test_suite_cleanup(void)
{
    test_suite_cleanup(&test_context);

    return 0;
}

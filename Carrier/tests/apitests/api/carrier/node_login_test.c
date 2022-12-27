#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include <crystal.h>

#include "ela_carrier.h"

#include "config.h"
#include "cond.h"
#include "test_helper.h"

static void ready_cb(ElaCarrier *c, void *context)
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

static CarrierContext carrier_context = {
    .cbs = &callbacks,
    .carrier = NULL,
    .ready_cond = &ready_cond,
    .cond = NULL,
    .friend_status_cond = NULL,
    .extra = NULL
};

static TestContext test_context = {
    .carrier = &carrier_context,
    .session = NULL,
    .stream  = NULL
};

static void test_node_login(void)
{
    ElaCarrier *carrier = test_context.carrier->carrier;
    char userid[ELA_MAX_ID_LEN + 1];
    char nodeid[ELA_MAX_ID_LEN + 1];
    char *p, *q;

    p = ela_get_userid(carrier, userid, sizeof(userid));
    CU_ASSERT_PTR_NOT_NULL_FATAL(p);

    q = ela_get_nodeid(carrier, nodeid, sizeof(nodeid));
    CU_ASSERT_PTR_NOT_NULL_FATAL(q);

    CU_ASSERT_STRING_EQUAL(p, q);
}

static CU_TestInfo cases[] = {
    { "test_node_login", test_node_login },
    { NULL, NULL }
};

CU_TestInfo *node_login_test_get_cases(void)
{
    return cases;
}

int node_login_test_suite_init(void)
{
    int rc;
    char path[128] = {0};

    snprintf(path, 128, "%s/tests/.carrier.pref",
            global_config.shared_options.persistent_location);
    unlink(path);

    rc = test_suite_init_ext(&test_context, true);
    if (rc < 0)
        CU_FAIL("Error: test suite initialize error");

    return rc;
}

int node_login_test_suite_cleanup(void)
{
    char path[128] = {0};

    test_suite_cleanup(&test_context);

    snprintf(path, 128, "%s/tests/.carrier.pref",
            global_config.shared_options.persistent_location);
    unlink(path);

    return 0;
}

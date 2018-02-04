#ifndef __API_CARRIER_TEST_SUITES_H__
#define __API_CARRIER_TEST_SUITES_H__

DECL_TESTSUITE(check_id_test)
DECL_TESTSUITE(get_id_test)
DECL_TESTSUITE(get_info_test)
DECL_TESTSUITE(friend_request_test)
DECL_TESTSUITE(friend_label_test)
DECL_TESTSUITE(friend_message_test)
DECL_TESTSUITE(friend_invite_test)

#define DEFINE_CARRIER_TESTSUITES \
    DEFINE_TESTSUITE(check_id_test), \
    DEFINE_TESTSUITE(get_id_test), \
    DEFINE_TESTSUITE(get_info_test), \
    DEFINE_TESTSUITE(friend_request_test), \
    DEFINE_TESTSUITE(friend_label_test), \
    DEFINE_TESTSUITE(friend_message_test),\
    DEFINE_TESTSUITE(friend_invite_test)

#endif /* __API_CARRIER_TEST_SUITES_H__ */

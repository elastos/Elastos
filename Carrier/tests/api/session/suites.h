#ifndef __API_SESSION_TEST_SUITES_H__
#define __API_SESSION_TEST_SUITES_H__

DECL_TESTSUITE(session_new_test)
DECL_TESTSUITE(session_request_reply_test)
DECL_TESTSUITE(session_stream_test)
DECL_TESTSUITE(session_stream_type_test)
DECL_TESTSUITE(session_stream_state_test)
DECL_TESTSUITE(session_channel_test)
DECL_TESTSUITE(session_portforwarding_test)

#define DEFINE_SESSION_TESTSUITES \
    DEFINE_TESTSUITE(session_new_test), \
    DEFINE_TESTSUITE(session_request_reply_test), \
    DEFINE_TESTSUITE(session_stream_type_test), \
    DEFINE_TESTSUITE(session_stream_state_test), \
    DEFINE_TESTSUITE(session_stream_test), \
    DEFINE_TESTSUITE(session_channel_test), \
    DEFINE_TESTSUITE(session_portforwarding_test)

#endif /* __API_SESSION_TEST_SUITES_H__ */

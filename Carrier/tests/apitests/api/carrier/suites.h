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

#ifndef __API_CARRIER_TEST_SUITES_H__
#define __API_CARRIER_TEST_SUITES_H__

DECL_TESTSUITE(check_id_test)
DECL_TESTSUITE(check_api_args_test)
DECL_TESTSUITE(get_id_test)
DECL_TESTSUITE(get_info_test)
DECL_TESTSUITE(friend_request_test)
DECL_TESTSUITE(friend_label_test)
DECL_TESTSUITE(friend_message_test)
DECL_TESTSUITE(friend_offline_message_test)
DECL_TESTSUITE(friend_receipt_message_test)
DECL_TESTSUITE(friend_invite_test)
DECL_TESTSUITE(friend_invite_assembly_test)
DECL_TESTSUITE(group_new_test)
DECL_TESTSUITE(group_invite_join_test)
DECL_TESTSUITE(group_message_test)
DECL_TESTSUITE(group_title_test)
DECL_TESTSUITE(group_peer_test)
DECL_TESTSUITE(group_list_test)

#define DEFINE_CARRIER_TESTSUITES \
    DEFINE_TESTSUITE(check_id_test), \
    DEFINE_TESTSUITE(check_api_args_test), \
    DEFINE_TESTSUITE(get_id_test), \
    DEFINE_TESTSUITE(get_info_test), \
    DEFINE_TESTSUITE(friend_request_test), \
    DEFINE_TESTSUITE(friend_label_test), \
    DEFINE_TESTSUITE(friend_message_test), \
    DEFINE_TESTSUITE(friend_offline_message_test), \
    DEFINE_TESTSUITE(friend_receipt_message_test), \
    DEFINE_TESTSUITE(friend_invite_test), \
    DEFINE_TESTSUITE(friend_invite_assembly_test), \
    DEFINE_TESTSUITE(group_new_test), \
    DEFINE_TESTSUITE(group_invite_join_test), \
    DEFINE_TESTSUITE(group_message_test), \
    DEFINE_TESTSUITE(group_title_test), \
    DEFINE_TESTSUITE(group_peer_test), \
    DEFINE_TESTSUITE(group_list_test)

#endif /* __API_CARRIER_TEST_SUITES_H__ */

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

#ifndef __TEST_HELPER_H__
#define __TEST_HELPER_H__

#include "ela_carrier.h"
#include "ela_session.h"

#include "test_context.h"

#define FREE_ANYWAY(ptr) do {   \
    if ((ptr)) {                \
        free(ptr);              \
        (ptr) = NULL;           \
    }                           \
} while(0)

extern char robotid[ELA_MAX_ID_LEN + 1];

extern char robotaddr[ELA_MAX_ADDRESS_LEN + 1];

int test_suite_init_ext(TestContext *ctx, bool udp_disabled);

int test_suite_init(TestContext *ctx);

int test_suite_cleanup(TestContext *ctx);

int add_friend_anyway(TestContext *ctx, const char *userid, const char *address);

int remove_friend_anyway(TestContext *ctx, const char *userid);

int robot_sinit(void);

void robot_sfree(void);

const char *stream_state_name(ElaStreamState state);

void test_stream_scheme(ElaStreamType stream_type, int stream_options,
                        TestContext *context, int (*do_work_cb)(TestContext *));

const char* connection_str(enum ElaConnectionStatus status);

void test_group_scheme(TestContext *context, 
                       int (*do_work_cb)(TestContext *));

void test_filetransfer_scheme(TestContext *context, int (*do_work_cb)(TestContext *),
                              bool use_ft_info);

int write_cmd(const char *cmd, ...);
int read_ack(const char *format, ...);

#endif /* __TEST_HELPER_H__ */

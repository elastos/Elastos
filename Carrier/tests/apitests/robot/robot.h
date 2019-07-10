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

#ifndef __ROBOT_H__
#define __ROBOT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ela_carrier.h"
#include "ela_session.h"
#include "test_context.h"

enum {
    OffMsgCase_Zero = 0,
    OffMsgCase_Single = 1,
    OffMsgCase_Bulk = 2
};

struct CarrierContextExtra {
    pthread_t tid;
    pthread_mutex_t mutex;
    char userid[ELA_MAX_ID_LEN + 1];
    char *bundle;
    char *data;
    int len;
    int test_offmsg;
    int test_offmsg_count;
    int expected_offmsg_count;
    struct timeval test_offmsg_expires;
    char offmsg_header[32];
    char gcookie[128];
    int gcookie_len;
    char gfrom[ELA_MAX_ID_LEN + 1];
    char groupid[ELA_MAX_ID_LEN + 1];
    char fileid[ELA_MAX_FILE_ID_LEN + 1];
    char recv_file[ELA_MAX_FILE_NAME_LEN + 1];
};

#ifdef __cplusplus
}
#endif

#endif

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

#ifndef __TEST_CONTEXT_H__
#define __TEST_CONTEXT_H__

#include "ela_carrier.h"
#include "ela_session.h"

#include "cond.h"

typedef struct Condition Condition;
typedef struct CarrierContextExtra CarrierContextExtra;
typedef struct SessionContextExtra SessionContextExtra;
typedef struct StreamContextExtra  StreamContextExtra;

typedef struct StreamContext {
    ElaStreamCallbacks *cbs;
    int stream_id;
    ElaStreamState state;
    uint8_t state_bits;
    Condition *cond;

    StreamContextExtra *extra;
} StreamContext;

typedef struct SessionContext {
    ElaSessionRequestCallback *request_cb;
    int request_received;
    Condition *request_cond;

    ElaSessionRequestCompleteCallback *request_complete_cb;
    int request_complete_status;
    Condition *request_complete_cond;

    ElaSession *session;

    SessionContextExtra *extra;
} SessionContext;

typedef struct CarrierContext {
    ElaCallbacks *cbs;
    ElaCarrier *carrier;
    Condition *ready_cond;
    Condition *cond;
    Condition *friend_status_cond;
    pthread_t thread;
    /**
     * the sufficient and necessary condition of friend_status being "online" is:
     * 1. we are connected to carrier network
     * 2. peer is a friend of ours
     * 3. we received peer online notification
     */
    enum {
       OFFLINE,
       ONLINE,
       FAILED
    } friend_status;

    CarrierContextExtra *extra;
} CarrierContext;

typedef struct TestContext TestContext;

struct TestContext {
    CarrierContext *carrier;
    SessionContext *session;
    StreamContext  *stream;

    void (*context_reset)(TestContext *);
};

#endif /* __TEST_CONTEXT_H__ */

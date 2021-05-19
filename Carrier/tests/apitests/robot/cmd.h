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

#ifndef __ROBOT_CMD_H__
#define __ROBOT_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ela_carrier.h"
#include "ela_session.h"
#include "test_context.h"

#define MAX_CHANNEL_COUNT   256

int start_cmd_listener(const char *host, const char *port);
void stop_cmd_listener(void);
char *read_cmd(void);
void do_cmd(TestContext*, char*);
void faccept(TestContext *context, int argc, char *argv[]);

int write_ack(const char *what, ...);

extern CarrierContext carrier_context;
extern TestContext test_context;

#ifdef __cplusplus
}
#endif

#endif

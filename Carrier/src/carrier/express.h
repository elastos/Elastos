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
#ifndef __ELASTOS_CARRIER_EXPRESS__
#define __ELASTOS_CARRIER_EXPRESS__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ExpConnector     ExpressConnector;
typedef void (*ExpressOnRecvCallback)(ElaCarrier *carrier,
                                      const char *from,
                                      const uint8_t *data, size_t size,
                                      int64_t timestamp);
typedef void (*ExpressOnStatCallback)(ElaCarrier *carrier,
                                      const char *from,
                                      int64_t msgid, int errcode);

ExpressConnector *express_connector_create(ElaCarrier *carrier,
                                           ExpressOnRecvCallback on_msg_cb,
                                           ExpressOnRecvCallback on_req_cb,
                                           ExpressOnStatCallback on_stat_cb);

void express_connector_kill(ExpressConnector *connector);

int express_enqueue_post_request(ExpressConnector *connector,
                                 const char *address,
                                 const void *data, size_t size);

int express_enqueue_post_message(ExpressConnector *connector,
                                 const char *friendid,
                                 const void *data, size_t size);

int express_enqueue_post_message_with_receipt(ExpressConnector *connector,
                                              const char *friendid,
                                              const void *data, size_t size,
                                              int64_t msgid);

int express_enqueue_pull_messages(ExpressConnector *connector);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__ELASTOS_CARRIER_EXPRESS__

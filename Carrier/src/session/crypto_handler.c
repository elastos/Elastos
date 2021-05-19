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

#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>

#include <crystal.h>

#include "flex_buffer.h"
#include "session.h"
#include "stream_handler.h"

typedef struct CryptoHandler {
    StreamHandler base;
} CryptoHandler;

static
ssize_t crypto_handler_write(StreamHandler *handler, FlexBuffer *buf)
{
    ElaSession *ws = handler->stream->session;
    FlexBuffer *cipher_buf;
    ssize_t cipher_len;
    ssize_t written;

    assert(handler);
    assert(handler->next);
    assert(buf);
    assert(flex_buffer_offset(buf) >= ZERO_BYTES);

    flex_buffer_alloca(cipher_buf, flex_buffer_size(buf) + FLEX_PADDING_LEN,
                       FLEX_PADDING_LEN - ZERO_BYTES);

    flex_buffer_backward_offset(buf, ZERO_BYTES);

    cipher_len  = crypto_encrypt2(ws->crypto.key, ws->nonce,
                                  flex_buffer_mutable_ptr(buf),
                                  flex_buffer_size(buf),
                                  flex_buffer_mutable_ptr(cipher_buf));

    // Reset plain buf's offset
    flex_buffer_forward_offset(buf, ZERO_BYTES);

    if (cipher_len <= 0) {
        vlogE("Stream: %d crypto handler encrypt data error.",
              handler->stream->id);
        return ELA_GENERAL_ERROR(ELAERR_ENCRYPT);
    } else {
        vlogT("Stream: %d crypto handler encrypted %zu bytes data.",
              handler->stream->id, cipher_len - ZERO_BYTES);

        flex_buffer_set_size(cipher_buf, cipher_len);
        flex_buffer_forward_offset(cipher_buf, ZERO_BYTES - MAC_BYTES);

        written = handler->next->write(handler->next, cipher_buf);

        return written == flex_buffer_size(cipher_buf) ?
                                flex_buffer_size(buf) : written;
    }
}

static
void crypto_handler_on_rx_data(StreamHandler *handler, FlexBuffer *buf)
{
    ElaSession *ws = handler->stream->session;
    FlexBuffer *plain_buf;
    ssize_t plain_len;

    assert(handler);
    assert(handler->prev);
    assert(buf);
    assert(flex_buffer_offset(buf) >= (ZERO_BYTES - MAC_BYTES));

    flex_buffer_alloca(plain_buf, flex_buffer_size(buf) + FLEX_PADDING_LEN,
                       FLEX_PADDING_LEN - (ZERO_BYTES - MAC_BYTES));

    flex_buffer_backward_offset(buf, ZERO_BYTES - MAC_BYTES);

    plain_len = crypto_decrypt2(ws->crypto.key, ws->nonce,
                                flex_buffer_mutable_ptr(buf),
                                flex_buffer_size(buf),
                                flex_buffer_mutable_ptr(plain_buf));

    // Reset cipher buf's offset
    flex_buffer_forward_offset(buf, ZERO_BYTES - MAC_BYTES);

    if (plain_len <=0) {
        vlogE("Stream: %d crypto handler decrypt data error.",
              handler->stream->id);
        // TODO: need to stop stream or fire failed state.
        return;
    } else {
        vlogT("Stream: %d crypto handler decrypt %zu bytes data.",
              handler->stream->id, plain_len - ZERO_BYTES);

        flex_buffer_set_size(plain_buf, plain_len);
        flex_buffer_forward_offset(plain_buf, ZERO_BYTES);

        handler->prev->on_data(handler->prev, plain_buf);
    }
}

static void crypto_handler_destroy(void *p)
{
    CryptoHandler *handler = (CryptoHandler *)p;

    if (handler->base.next)
        deref(handler->base.next);

    vlogD("Stream: %d crypto handler destroyed.", handler->base.stream->id);
}

int crypto_handler_create(ElaStream *s, StreamHandler **handler)
{
    CryptoHandler *_handler = NULL;

    _handler = (CryptoHandler *)rc_zalloc(sizeof(CryptoHandler), crypto_handler_destroy);
    if (!_handler)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    _handler->base.name = "Crypto Handler";
    _handler->base.stream = s;

    _handler->base.init    = default_handler_init;
    _handler->base.prepare = default_handler_prepare;
    _handler->base.start   = default_handler_start;
    _handler->base.stop    = default_handler_stop;
    _handler->base.write   = crypto_handler_write;
    _handler->base.on_data = crypto_handler_on_rx_data;
    _handler->base.on_state_changed = default_handler_on_state_changed;

    vlogD("Stream: %d crypto handler created", s->id);

    *handler = (StreamHandler *)_handler;
    return 0;
}

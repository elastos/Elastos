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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if defined(__APPLE__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif
#include <tox/tox.h>
#if defined(__APPLE__)
#pragma GCC diagnostic pop
#endif

#include <vlog.h>

#include "dht.h"
#include "dht_callbacks.h"

#include "ela_carrier.h"

const char *data_filename = "dhtdata";

struct DHT {
    Tox *tox;
};

static inline int __dht_new_error(TOX_ERR_NEW code)
{
    int rc;

    switch (code) {
    case TOX_ERR_NEW_OK:
        rc = ELASUCCESS;
        break;

    case TOX_ERR_NEW_NULL:
        rc = ELA_DHT_ERROR(ELAERR_INVALID_ARGS);
        break;

    case TOX_ERR_NEW_MALLOC:
        rc = ELA_DHT_ERROR(ELAERR_OUT_OF_MEMORY);
        break;

    case TOX_ERR_NEW_PORT_ALLOC:
        rc = ELA_DHT_ERROR(ELAERR_PORT_ALLOC);
        break;

    case TOX_ERR_NEW_PROXY_BAD_TYPE:
        rc = ELA_DHT_ERROR(ELAERR_BAD_PROXY_TYPE);
        break;

    case TOX_ERR_NEW_PROXY_BAD_HOST:
        rc = ELA_DHT_ERROR(ELAERR_BAD_PROXY_HOST);
        break;

    case TOX_ERR_NEW_PROXY_BAD_PORT:
        rc = ELA_DHT_ERROR(ELAERR_BAD_PROXY_PORT);
        break;

    case TOX_ERR_NEW_PROXY_NOT_FOUND:
        rc = ELA_DHT_ERROR(ELAERR_PROXY_NOT_AVAILABLE);
        break;

    case TOX_ERR_NEW_LOAD_ENCRYPTED:
        rc = ELA_DHT_ERROR(ELAERR_ENCRYPTED_PERSISTENT_DATA);
        break;

    case TOX_ERR_NEW_LOAD_BAD_FORMAT:
        rc = ELA_DHT_ERROR(ELAERR_BAD_PERSISTENT_DATA);
        break;

    default:
        rc = ELA_DHT_ERROR(ELAERR_UNKNOWN);
    }

    return rc;
}

static inline int __dht_bootstrap_error(TOX_ERR_BOOTSTRAP code)
{
    int rc;

    switch (code) {
    case TOX_ERR_BOOTSTRAP_OK:
        rc = ELASUCCESS;
        break;

    case TOX_ERR_BOOTSTRAP_NULL:
        rc = ELA_DHT_ERROR(ELAERR_INVALID_ARGS);
        break;

    case TOX_ERR_BOOTSTRAP_BAD_HOST:
        rc = ELA_DHT_ERROR(ELAERR_BAD_BOOTSTRAP_HOST);
        break;

    case TOX_ERR_BOOTSTRAP_BAD_PORT:
        rc = ELA_DHT_ERROR(ELAERR_BAD_BOOTSTRAP_PORT);
        break;

    default:
        rc = ELA_DHT_ERROR(ELAERR_UNKNOWN);
    }

    return rc;
}

static inline int __dht_friend_query_error(TOX_ERR_FRIEND_QUERY code)
{
    int rc;

    switch (code) {
    case TOX_ERR_FRIEND_QUERY_OK:
        rc = ELASUCCESS;
        break;

    case TOX_ERR_FRIEND_QUERY_NULL:
        rc = ELA_DHT_ERROR(ELAERR_INVALID_ARGS);
        break;

    case TOX_ERR_FRIEND_QUERY_FRIEND_NOT_FOUND:
        rc = ELA_DHT_ERROR(ELAERR_NOT_EXIST);
        break;

    default:
        rc = ELA_DHT_ERROR(ELAERR_UNKNOWN);
    }

    return rc;
}

static inline int __dht_friend_get_pk_error(TOX_ERR_FRIEND_GET_PUBLIC_KEY code)
{
    int rc;

    switch (code) {
    case TOX_ERR_FRIEND_GET_PUBLIC_KEY_OK:
        rc = ELASUCCESS;
        break;

    case TOX_ERR_FRIEND_GET_PUBLIC_KEY_FRIEND_NOT_FOUND:
        rc = ELA_DHT_ERROR(ELAERR_NOT_EXIST);
        break;

    default:
        rc = ELA_DHT_ERROR(ELAERR_UNKNOWN);
    }

    return rc;
}

static inline int __dht_set_info_error(TOX_ERR_SET_INFO code)
{
    int rc;

    switch (code) {
    case TOX_ERR_SET_INFO_OK:
        rc = ELASUCCESS;
        break;

    case TOX_ERR_SET_INFO_NULL:
        rc = ELA_DHT_ERROR(ELAERR_INVALID_ARGS);
        break;

    case TOX_ERR_SET_INFO_TOO_LONG:
        rc = ELA_DHT_ERROR(ELAERR_TOO_LONG);
        break;

    default:
        rc = ELA_DHT_ERROR(ELAERR_UNKNOWN);
    }

    return rc;
}

static inline int __dht_friend_by_pk_error(TOX_ERR_FRIEND_BY_PUBLIC_KEY code)
{
    int rc;

    switch (code) {
    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_OK:
        rc = ELASUCCESS;
        break;

    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_NULL:
        rc = ELA_DHT_ERROR(ELAERR_INVALID_ARGS);
        break;

    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_NOT_FOUND:
        rc = ELA_DHT_ERROR(ELAERR_NOT_EXIST);
        break;

    default:
        rc = ELA_DHT_ERROR(ELAERR_UNKNOWN);
    }

    return rc;
}

static inline int __dht_friend_add_error(TOX_ERR_FRIEND_ADD code)
{
    int rc;

    switch (code) {
    case TOX_ERR_FRIEND_ADD_OK:
        rc = ELASUCCESS;
        break;

    case TOX_ERR_FRIEND_ADD_NULL:
        rc = ELA_DHT_ERROR(ELAERR_INVALID_ARGS);
        break;

    case TOX_ERR_FRIEND_ADD_TOO_LONG:
        rc = ELA_DHT_ERROR(ELAERR_TOO_LONG);
        break;

    case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
        rc = ELA_DHT_ERROR(ELAERR_INVALID_ARGS);
        break;

    case TOX_ERR_FRIEND_ADD_OWN_KEY:
        rc = ELA_DHT_ERROR(ELAERR_ADD_SELF);
        break;

    case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
        rc = ELA_DHT_ERROR(ELAERR_ALREADY_EXIST);
        break;

    case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
    case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
        rc = ELA_DHT_ERROR(ELAERR_BAD_ADDRESS);
        break;

    case TOX_ERR_FRIEND_ADD_MALLOC:
        rc = ELA_DHT_ERROR(ELAERR_OUT_OF_MEMORY);
        break;

    default:
        rc = ELA_DHT_ERROR(ELAERR_UNKNOWN);
    }

    return rc;
}

static inline int __dht_friend_send_msg_error(TOX_ERR_FRIEND_SEND_MESSAGE code)
{
    int rc;

    switch (code) {
    case TOX_ERR_FRIEND_SEND_MESSAGE_OK:
        rc = ELASUCCESS;
        break;

    case TOX_ERR_FRIEND_SEND_MESSAGE_NULL:
        rc = ELA_DHT_ERROR(ELAERR_INVALID_ARGS);
        break;

    case TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_FOUND:
        rc = ELA_DHT_ERROR(ELAERR_NOT_EXIST);
        break;

    case TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED:
        rc = ELA_DHT_ERROR(ELAERR_FRIEND_OFFLINE);
        break;

    case TOX_ERR_FRIEND_SEND_MESSAGE_SENDQ:
        rc = ELA_DHT_ERROR(ELAERR_OUT_OF_MEMORY);
        break;

    case TOX_ERR_FRIEND_SEND_MESSAGE_TOO_LONG:
        rc = ELA_DHT_ERROR(ELAERR_TOO_LONG);
        break;

    case TOX_ERR_FRIEND_SEND_MESSAGE_EMPTY:
        rc = ELA_DHT_ERROR(ELAERR_INVALID_ARGS);
        break;

    default:
        rc = ELA_DHT_ERROR(ELAERR_UNKNOWN);
    }

    return rc;
}

static inline int __dht_friend_delete_error(TOX_ERR_FRIEND_DELETE code)
{
    int rc;

    switch (code) {
    case TOX_ERR_FRIEND_DELETE_OK:
        rc = ELASUCCESS;
        break;

    case TOX_ERR_FRIEND_DELETE_FRIEND_NOT_FOUND:
        rc = ELA_DHT_ERROR(ELAERR_NOT_EXIST);
        break;

    default:
        rc = ELA_DHT_ERROR(ELAERR_UNKNOWN);
    }

    return rc;
}

static bool is_connected(TOX_CONNECTION connection)
{
    bool is_connected;

    switch(connection) {
    case TOX_CONNECTION_NONE:
    default:
        is_connected = false;
        break;
    case TOX_CONNECTION_TCP:
    case TOX_CONNECTION_UDP:
        is_connected = true;
        break;
    }

    return is_connected;
}

static
void notify_connection_cb(Tox *tox, TOX_CONNECTION connection, void *context)
{
    DHTCallbacks *cbs = (DHTCallbacks *)context;
    cbs->notify_connection(is_connected(connection), cbs->context);
}

static
void notify_friend_status_message_cb(Tox *tox, uint32_t friend_number,
                                     const uint8_t *status_message,
                                     size_t length, void *context)
{
    DHTCallbacks *cbs = (DHTCallbacks *)context;
    cbs->notify_friend_desc(friend_number, status_message, length, cbs->context);
}

static
void notify_friend_connection_cb(Tox *tox, uint32_t friend_number,
                                 TOX_CONNECTION connection, void *context)
{
    DHTCallbacks *cbs = (DHTCallbacks *)context;
    cbs->notify_friend_connection(friend_number, is_connected(connection),
                                  cbs->context);
}

static
void notify_friend_status_cb(Tox *tox, uint32_t friend_number,
                             TOX_USER_STATUS status, void *context)
{
    DHTCallbacks *cbs = (DHTCallbacks *)context;
    cbs->notify_friend_status(friend_number, (int)status, cbs->context);
}

static
void notify_friend_request_cb(Tox *tox, const uint8_t *pubic_key,
                              const uint8_t *hello, size_t length,
                              void *context)
{
    DHTCallbacks *cbs = (DHTCallbacks *)context;
    cbs->notify_friend_request(pubic_key, hello, length, cbs->context);
}

static
void notify_friend_message_cb(Tox *tox, uint32_t friend_number,
                              TOX_MESSAGE_TYPE type,
                              const uint8_t *message, size_t length,
                              void *context)
{
    DHTCallbacks *cbs = (DHTCallbacks *)context;

    if (type != TOX_MESSAGE_TYPE_NORMAL)
        return;

    cbs->notify_friend_message(friend_number, message, length, cbs->context);
}

static
void log_cb(Tox *tox, TOX_LOG_LEVEL level, const char *file, uint32_t line,
            const char *func, const char *message, void *user_data)
{
    char *buf;
    size_t len;
    int _level;

    switch(level) {
    case TOX_LOG_LEVEL_TRACE:
    default:
        _level = VLOG_TRACE;
        break;
    case TOX_LOG_LEVEL_DEBUG:
        _level = VLOG_DEBUG;
        break;
    case TOX_LOG_LEVEL_INFO:
        _level = VLOG_INFO;
        break;
    case TOX_LOG_LEVEL_WARNING:
        _level = VLOG_WARN;
        break;
    case TOX_LOG_LEVEL_ERROR:
        _level = VLOG_ERROR;
        break;
    }

    len = strlen(file) + sizeof(uint32_t) + strlen(func) + strlen(message) + 8;
    buf = (char *)alloca(len);

    sprintf(buf, "<%s>:%s\n", func, message);
    vlog(_level, "%s", buf);
}

int dht_new(const uint8_t *savedata, size_t datalen, bool udp_enabled, DHT *dht)
{
    struct Tox_Options options;
    TOX_ERR_NEW error;
    Tox *tox;

    assert(dht);

    tox_options_default(&options);
    options.local_discovery_enabled = true;
    options.ipv6_enabled = false;
    options.udp_enabled = udp_enabled;
    options.log_callback = log_cb;

    if (savedata && datalen) {
        options.savedata_type = TOX_SAVEDATA_TYPE_TOX_SAVE;
        options.savedata_data = savedata;
        options.savedata_length = datalen;
    } else {
        options.savedata_type = TOX_SAVEDATA_TYPE_NONE;
        options.savedata_data = NULL;
        options.savedata_length = 0;
    }

    tox = tox_new(&options, &error);
    if (!tox) {
        vlogE("DHT: new dht error (%d).", error);
        return __dht_new_error(error);
    }

    tox_callback_self_connection_status(tox, notify_connection_cb);
    tox_callback_friend_status_message(tox, notify_friend_status_message_cb);
    tox_callback_friend_connection_status(tox, notify_friend_connection_cb);
    tox_callback_friend_status(tox, notify_friend_status_cb);
    tox_callback_friend_request(tox, notify_friend_request_cb);
    tox_callback_friend_message(tox, notify_friend_message_cb);

    dht->tox = tox;

    return 0;
}

void dht_kill(DHT *dht)
{
    Tox *tox = dht->tox;

    if (!tox)
        return;

    tox_kill(tox);
}

/*
 * The length of array address must be at least DHT_PUBLIC_KEY_SIZE.
 */
int _dht_bootstrap(DHT *dht, const char *ipv4, const char *ipv6, int port,
                  const uint8_t *address)
{
    Tox *tox = dht->tox;
    bool success;
    TOX_ERR_BOOTSTRAP error;

    assert(dht);
    assert(*ipv4 || *ipv6);
    assert(port > 0);
    assert(address);

    if (*ipv4) {
        success = tox_bootstrap(tox, ipv4, (uint16_t)port, address, &error);
        if (!success) {
            vlogE("DHT: add bootstrap %s:%d error (%d).", ipv4, port, error);
            return __dht_bootstrap_error(error);
        }

        success = tox_add_tcp_relay(tox, ipv4, (uint16_t)port, address, &error);
        if (!success)  {
            vlogE("DHT: add tcp relay %s:%d error (%d).", ipv4, port, error);
            return __dht_bootstrap_error(error);
        }
    }

    if (*ipv6) {
        success = tox_bootstrap(tox, ipv6, (uint16_t)port, address, &error);
        if (!success) {
            vlogE("DHT: add bootstrap %s:%d error (%d).", ipv6, port, error);
            return __dht_bootstrap_error(error);
        }

        success = tox_add_tcp_relay(tox, ipv6, (uint16_t)port, address, &error);
        if (!success)  {
            vlogE("DHT: add tcp relay %s:%d error (%d).", ipv6, port, error);
            return __dht_bootstrap_error(error);
        }
    }

    return 0;
}

void dht_self_set_nospam(DHT *dht, uint32_t nospam)
{
    Tox *tox = dht->tox;

    assert(tox);

    tox_self_set_nospam(tox, nospam);
}

uint32_t dht_self_get_nospam(DHT *dht)
{
    Tox *tox = dht->tox;

    assert(tox);

    return tox_self_get_nospam(tox);
}

void dht_self_get_secret_key(DHT *dht, uint8_t *secret_key)
{
    Tox *tox = dht->tox;

    assert(tox);

    tox_self_get_secret_key(tox, secret_key);
}

void dht_self_set_status(DHT *dht, int status)
{
    Tox *tox = dht->tox;

    assert(tox);

    tox_self_set_status(tox, (TOX_USER_STATUS)status);
}

int dht_self_get_status(DHT *dht)
{
    Tox *tox = dht->tox;

    assert(tox);

    return (int)tox_self_get_status(tox);
}

int dht_get_self_info(DHT *dht, SelfInfoCallback cb, void *context)
{
    Tox *tox = dht->tox;
    uint8_t address[DHT_ADDRESS_SIZE];
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
    size_t desc_len;
    uint8_t *desc;
    TOX_USER_STATUS user_status;

    assert(tox);
    assert(cb);

    tox_self_get_address(tox, address);
    tox_self_get_public_key(tox, public_key);

    desc_len = tox_self_get_status_message_size(tox);

    if (desc_len > 0) {
        desc = alloca(desc_len);
        tox_self_get_status_message(tox, desc);
    } else {
        desc = alloca(1);
    }

    user_status = tox_self_get_status(tox);

    cb(address, public_key, (int)user_status, desc, desc_len, context);

    return 0;
}

int dht_get_friends(DHT *dht, FriendsIterateCallback cb, void *context)
{
    Tox *tox = dht->tox;
    size_t list_sz;
    uint32_t *friend_list;
    int i;
    uint8_t desc[TOX_MAX_STATUS_MESSAGE_LENGTH];
    uint8_t public_key[TOX_PUBLIC_KEY_SIZE];

    assert(tox);

    list_sz = tox_self_get_friend_list_size(tox);
    if (!list_sz)
        return 0;

    friend_list = (uint32_t *)alloca(list_sz * sizeof(uint32_t));
    tox_self_get_friend_list(tox, friend_list);

    for (i = 0; i < list_sz; i++) {
        size_t desc_len;
        bool success;
        TOX_ERR_FRIEND_QUERY error;
        TOX_USER_STATUS user_status;
        TOX_ERR_FRIEND_GET_PUBLIC_KEY _error;

        desc_len = tox_friend_get_status_message_size(tox, friend_list[i],
                                                      &error);
        if (error != TOX_ERR_FRIEND_QUERY_OK) {
            vlogE("DHT: get friend status message size error (%d).", error);
            return __dht_friend_query_error(error);
        }

        success = tox_friend_get_status_message(tox, friend_list[i], desc,
                                                &error);
        if (!success) {
            vlogE("DHT: get friend status message error (%d).", error);
            return __dht_friend_query_error(error);
        }

        user_status = tox_friend_get_status(tox, friend_list[i], &error);
        if (error != TOX_ERR_FRIEND_QUERY_OK) {
            vlogE("DHT: get friend user status error (%d).", error);
            return __dht_friend_query_error(error);
        }

        success = tox_friend_get_public_key(tox, friend_list[i], public_key, &_error);
        if (!success) {
            vlogE("DHT: get friend public key error (%d).", _error);
            return __dht_friend_get_pk_error(_error);
        }

        success = cb(friend_list[i], public_key, (int)user_status, desc, desc_len,
                     context);
        if (!success)
            return 0;
    }

    return 0;
}

size_t dht_get_savedata_size(DHT *dht)
{
    Tox *tox = dht->tox;

    assert(tox);

    return tox_get_savedata_size(tox);
}

void dht_get_savedata(DHT *dht, uint8_t *data)
{
    Tox *tox = dht->tox;

    assert(tox);

    tox_get_savedata(tox, data);
}

int dht_iteration_idle(DHT *dht)
{
    Tox *tox = dht->tox;

    assert(tox);

    return (int)tox_iteration_interval(tox);
}

void dht_iterate(DHT *dht, void *context)
{
    Tox *tox = dht->tox;

    assert(tox);

    tox_iterate(tox, context);
}

int dht_self_set_name(DHT *dht, uint8_t *name, size_t length)
{
    Tox *tox = dht->tox;
    TOX_ERR_SET_INFO error;
    bool success;

    assert(tox);
    assert(name);

    success = tox_self_set_name(tox, name, length, &error);
    if (!success) {
        vlogE("DHT: set self name error (%d).", error);
        return __dht_set_info_error(error);
    }

    return 0;
}

int dht_self_set_desc(DHT *dht, uint8_t *desc, size_t length)
{
    Tox *tox = dht->tox;
    TOX_ERR_SET_INFO error;
    bool success;

    assert(tox);
    assert(desc);

    success = tox_self_set_status_message(tox, desc, length, &error);
    if (!success) {
        vlogE("DHT: set self description error (%d).", error);
        return __dht_set_info_error(error);
    }

    return 0;
}

int dht_get_friend_number(DHT *dht, const uint8_t *public_key,
                          uint32_t *friend_number)
{
    Tox *tox = dht->tox;
    TOX_ERR_FRIEND_BY_PUBLIC_KEY error;

    assert(tox);
    assert(public_key);
    assert(friend_number);

    *friend_number = tox_friend_by_public_key(tox, public_key, &error);
    if (*friend_number == UINT32_MAX)
        return __dht_friend_by_pk_error(error);
    else
        return 0;
}

int dht_friend_add(DHT *dht, const uint8_t *address, const uint8_t *msg,
                   size_t length, uint32_t *friend_number)
{
    Tox *tox = dht->tox;
    TOX_ERR_FRIEND_ADD error;
    uint32_t fid;

    assert(dht);
    assert(address);
    assert(msg && length > 0);

    fid = tox_friend_add(tox, address, msg, length, &error);
    if (fid == UINT32_MAX) {
        vlogW("DHT: add friend error (%d).", error);
        return __dht_friend_add_error(error);
    }

    if (friend_number)
        *friend_number = fid;

    return 0;
}

int dht_friend_add_norequest(DHT *dht, const uint8_t *public_key,
                             uint32_t *friend_number)
{
    Tox *tox = dht->tox;
    TOX_ERR_FRIEND_ADD error;
    uint32_t fid;

    assert(tox);
    assert(public_key);

    fid = tox_friend_add_norequest(tox, public_key, &error);
    if (fid == UINT32_MAX) {
        vlogE("DHT: add friend with no request error (%d).", error);
        return __dht_friend_add_error(error);
    }

    if (friend_number)
        *friend_number = fid;

    return 0;
}

int dht_friend_message(DHT *dht, uint32_t friend_number, const uint8_t *data,
                       size_t length)
{
    Tox *tox = dht->tox;
    TOX_ERR_FRIEND_SEND_MESSAGE error;

    assert(tox);
    assert(friend_number != UINT32_MAX);
    assert(data && length > 0);

    tox_friend_send_message(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL,
                            data, length, &error);
    if (error != TOX_ERR_FRIEND_SEND_MESSAGE_OK) {
        vlogW("DHT: send friend message to %u error (%d).", friend_number,
              error);
        return __dht_friend_send_msg_error(error);
    }

    return 0;
}

int dht_friend_delete(DHT *dht, uint32_t friend_number)
{
    Tox *tox = dht->tox;
    TOX_ERR_FRIEND_DELETE error;

    assert(tox);
    assert(friend_number != UINT32_MAX);

    tox_friend_delete(tox, friend_number, &error);
    if (error != TOX_ERR_FRIEND_DELETE_OK) {
        // vlogE("DHT: delete friend %d error (%d).", friend_number, error);
        return __dht_friend_delete_error(error);
    }

    return 0;
}

int dht_get_random_tcp_relay(DHT *dht, char *tcp_relay, size_t buflen,
                             uint8_t *public_key)
{
    Tox *tox = dht->tox;
    int rc;
    struct in_addr in_addr;
    char addr_buf[45];
    const char *addr;

    rc = tox_self_get_random_tcp_relay(tox, (uint8_t *)&in_addr, public_key);
    if (rc < 0) {
        vlogE("DHT: get random_tcp relay error or no tcp relay connected");
        return rc;
    }

    addr = inet_ntop(AF_INET, &in_addr,
        addr_buf, sizeof(addr_buf));

    if (!addr || strlen(addr) >= buflen)
        return -1;

    strcpy(tcp_relay, addr);

    return 0;
}

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
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#else
#include <sys/time.h>
#endif

#include <cjson/cJSON.h>
#include <crystal.h>

#include "dht.h"
#include "ela_carrier.h"
#include "ela_carrier_impl.h"
#include "elacp.h"
#include "express.h"
#include "http_client.h"

struct ExpConnector {
    ElaCarrier *carrier;
    ExpressOnRecvCallback on_msg_cb;
    ExpressOnRecvCallback on_req_cb;
    ExpressOnStatCallback on_stat_cb;

    pthread_mutex_t lock;
    pthread_cond_t cond;

    list_t *task_list;
    int stop_flag;

    http_client_t *http_client;
    uint8_t shared_key[SYMMETRIC_KEY_BYTES];
    const char* base_url;
    uint32_t magic_num;
};

typedef struct ExpTasklet {
    list_entry_t entry;
    ExpressConnector *connector;

    int (*runner)(struct ExpTasklet *);
} ExpTasklet;

typedef struct {
    ExpTasklet base;

    char to[ELA_MAX_ADDRESS_LEN + 1];
    int64_t msgid;
    size_t pos;
    size_t length;
    uint8_t data[0];
} ExpSendTasklet;

typedef struct {
    ExpTasklet base;

    int64_t last_timestamp;
    size_t pos;
    uint8_t *data;
} ExpPullTasklet;

static const int  EXP_CURLCODE_MASK    = 0x00001000;
static const int  EXP_HTTP_MAGICNUM    = 0xCA6EE595;
static const int  EXP_HTTP_MAGICSIZE   = 4;
static const int  EXP_HTTP_TIMEOUT     = 30 * 1000; // ms
#define  EXP_HTTP_URL_MAXSIZE 1024

static inline int conv_curlcode(int curlcode) {
    return (curlcode | EXP_CURLCODE_MASK);
}

static inline char *my_userid(ExpressConnector *connector)
{
    return connector->carrier->me.userid;
}

static int compute_sharedkey(ElaCarrier *carrier, uint8_t *pk, uint8_t *shared_key)
{
    uint8_t sk[SECRET_KEY_BYTES];

    dht_self_get_secret_key(&carrier->dht, sk);
    crypto_compute_symmetric_key(pk, sk, shared_key);

    return 0;
}

static int compute_friend_sharedkey(ElaCarrier *carrier, const char *friendid, uint8_t *shared_key)
{
    uint8_t friend_pk[PUBLIC_KEY_BYTES];
    ssize_t size;
    int rc;

    size = base58_decode(friendid, strlen(friendid), friend_pk, sizeof(friend_pk));
    if (size != sizeof(friend_pk))  {
        vlogE("Express: Decode base58 friendid %s error", friendid);
        return ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS);
    }

    rc = compute_sharedkey(carrier, friend_pk, shared_key);
    if (rc < 0) {
        vlogE("Express: compute shared key error(%x)", rc);
        return rc;
    }

    return 0;
}

static ssize_t encrypt_data(uint8_t *key,
                                uint8_t *plain_data, size_t plain_len,
                                uint8_t *crypted_data)
{
    ssize_t rc;
    uint8_t *nonce_ptr = crypted_data;
    uint8_t *data_ptr = crypted_data + NONCE_BYTES;

    crypto_random_nonce(nonce_ptr);
    rc = crypto_encrypt(key, nonce_ptr, plain_data, plain_len, data_ptr);
    if (rc < 0)
        return ELA_EXPRESS_ERROR(ELAERR_ENCRYPT);

    return rc + NONCE_BYTES;
}

static ssize_t decrypt_data(uint8_t *key,
                                uint8_t *crypted_data, size_t crypted_len,
                                uint8_t *plain_data)
{
    ssize_t rc;
    uint8_t *nonce_ptr = crypted_data;
    uint8_t *data_ptr = crypted_data + NONCE_BYTES;
    int data_sz = crypted_len - NONCE_BYTES;

    rc = crypto_decrypt(key, nonce_ptr, data_ptr, data_sz, plain_data);
    if (rc < 0)
        return ELA_EXPRESS_ERROR(ELAERR_ENCRYPT);

    return rc;
}

int find_magic(uint8_t *buf, size_t start, size_t end, uint32_t magic_num)
{
    uint8_t *magicnum_buf = (uint8_t*)&magic_num;
    size_t pos;
    int idx;

    for(pos = start; pos <= end - EXP_HTTP_MAGICSIZE; pos++) {
        for(idx = 0; idx < EXP_HTTP_MAGICSIZE; idx++) {
            if(buf[pos + idx] != magicnum_buf[idx]) {
                break;
            }
        }
        if(idx == EXP_HTTP_MAGICSIZE) {
            return pos;
        }
    }

    return -1;
}

static int process_message(ExpressConnector *connector,
                           uint8_t *data, size_t size,
                           int64_t *timestamp)
{
    int rc;
    ElaCPPullMsg pmsg;

    rc = elacp_decode_pullmsg(data, &pmsg);
    if (rc < 0) {
        vlogE("Express: decode pullmsg flatbuffer failed.(%x)", rc);
        return ELA_EXPRESS_ERROR(ELAERR_BAD_FLATBUFFER);
    }

    *timestamp = pmsg.timestamp;

    if (pmsg.type == 'M') {
        uint8_t shared_key[SYMMETRIC_KEY_BYTES];
        uint8_t *plain_data;
        int plain_size;

        vlogV("Express: recieved offline message at time %llu", pmsg.timestamp);
        if (!ela_is_friend(connector->carrier, pmsg.from)) {
            vlogE("Express: Friend message not frond friend, dropped.");
            return -1;
        }

        rc = compute_friend_sharedkey(connector->carrier, pmsg.from, shared_key);
        if (rc < 0) {
            vlogE("Express: compute message shared key error(%x)", rc);
            return rc;
        }

        plain_data = (uint8_t *)calloc(1, pmsg.payload_sz);
        if (!plain_data) {
            vlogW("Express: parse message failed.");
            return ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY);
        }

        rc = decrypt_data(shared_key, (uint8_t *)pmsg.payload, pmsg.payload_sz,
                          plain_data);
        if (rc < 0) {
            vlogW("Express: decrypt message data failed: (%x).", rc);
            free(plain_data);
            return rc;
        }
        plain_size = rc;

        connector->on_msg_cb(connector->carrier, pmsg.from,
                             plain_data, plain_size,
                             pmsg.timestamp);
        free(plain_data);
    } else if (pmsg.type == 'R') {
        vlogV("Express: recieved offline request at time %llu", pmsg.timestamp);
        char addr[ELA_MAX_ADDRESS_LEN + 1];
        ela_get_address(connector->carrier, addr, sizeof(addr));
        if (strcmp(pmsg.address, addr) != 0) {
            vlogE("Express: Friend request with unmatched address, dropped.");
            return -1;
        }

        connector->on_req_cb(connector->carrier, pmsg.from,
                             pmsg.payload, pmsg.payload_sz,
                             pmsg.timestamp);
    } else {
        vlogW("Express: recieved offline unknown type at time %llu", pmsg.timestamp);
    }

    return 0;
}

static int parse_msg(ExpressConnector *connector,
                         uint8_t *crypted_data, size_t crypted_size,
                         int64_t *timestamp)
{
    int rc;
    uint8_t *plain_data;
    int plain_size;

    plain_data = (uint8_t *)calloc(1, crypted_size);
    if (!plain_data) {
        vlogW("Express: parse message failed.");
        return ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY);
    }

    rc = decrypt_data(connector->shared_key,
                      crypted_data, crypted_size,
                      plain_data);
    if (rc < 0) {
        vlogW("Express: decrypt message data failed: (%x).", rc);
        free(plain_data);
        return rc;
    }
    plain_size = rc;

    rc = process_message(connector, plain_data, plain_size, timestamp);
    if (rc < 0) {
        vlogW("Express: process message failed: (%x).", rc);
        free(plain_data);
        return rc;
    }

    free(plain_data);
    return 0;
}

static int parse_msg_stream(ExpressConnector *connector,
                                uint8_t *data, size_t size,
                                int64_t *timestamp)
{
    int rc;
    int data_off, magic_off;
    uint8_t *msg;
    uint32_t msg_sz;

    data_off = 0;
    while(true) {
        // search magic number
        magic_off = find_magic(data, data_off, size, connector->magic_num);
        if(magic_off < 0) { // has no magic left
            break;
        }
        data_off = magic_off + EXP_HTTP_MAGICSIZE;

        // get message size
        msg_sz = ntohl(*(uint32_t*)&data[data_off]);
        data_off += sizeof(msg_sz);

        if(data_off + msg_sz > size) { // message is incomplete 
            break;
        }
        msg = &data[data_off];
        data_off += msg_sz;

        if(msg_sz == 0) { // empty message, ignore it
            continue;
        }
        rc = parse_msg(connector, msg, msg_sz, timestamp);
        if(rc < 0) {
            vlogW("Express: http response body parse message failed: (%x).",rc);
            continue; // skip to next message
        }
    }

    return data_off;
}

size_t http_read_data(char *buffer, size_t size, size_t nitems, void *userdata)
{
    int rc;
    ExpPullTasklet *task = (ExpPullTasklet *)userdata;
    ExpressConnector *connector = task->base.connector;
    size_t buf_sz = size * nitems;
    int parsed_sz;

    if(task->data == NULL) {
        task->data = rc_alloc(buf_sz, NULL);
    } else {
        task->data = rc_realloc(task->data, task->pos + buf_sz);
    }
    memcpy(task->data + task->pos, buffer, buf_sz);
    task->pos += buf_sz;

    rc = parse_msg_stream(connector, task->data, task->pos, &task->last_timestamp);
    if (rc < 0) {
        ela_set_error(rc);
    }
    parsed_sz = rc;

    if(parsed_sz > 0 && parsed_sz < (int)task->pos) {
        size_t remain_sz = task->pos - parsed_sz;
        uint8_t *remain_data = rc_alloc(remain_sz, NULL);
        memcpy(remain_data, task->data + parsed_sz, remain_sz);
        deref(task->data);
        task->data = remain_data;
        task->pos = remain_sz;
    } else {
        deref(task->data);
        task->data = NULL;
        task->pos = 0;
    }

    return buf_sz;
}

size_t http_write_data(char *buffer, size_t size, size_t nitems, void *userdata)
{
    ExpSendTasklet *task = (ExpSendTasklet *)userdata;
    int buf_sz = size * nitems;
    int data_sz = task->length - task->pos;

    if(data_sz <= 0) {
        return 0;
    }

    data_sz = (data_sz < buf_sz ? data_sz : buf_sz);
    memcpy(buffer, task->data + task->pos, data_sz);
    
    task->pos += data_sz;
    if(task->pos >= task->length) {
        task->pos = 0;
        task->length = 0;
    }

    return data_sz;
}

static int http_do(ExpressConnector *connector, http_client_t *http_client,
                       const char* path, http_method_t method,
                       void *userdata)
{
    int rc;
    char url[EXP_HTTP_URL_MAXSIZE];
    long http_client_rescode = 0;

    const char* dowhat = (method == HTTP_METHOD_POST ? "pushing"
                       : (method == HTTP_METHOD_GET ? "pulling"
                       : (method == HTTP_METHOD_DELETE ? "deleting"
                       : "unknown")));

    snprintf(url, sizeof(url), "%s/%s", connector->base_url, path);
    vlogD("Express: %s message, url: %s", dowhat, url);

    http_client_reset(http_client);

    rc = http_client_set_url(http_client, url);
    if(rc != 0) {
        vlogE("Express: Failed to set http url.(CURLE: %d)", rc);
        return ELA_EXPRESS_ERROR(conv_curlcode(rc));
    }

    rc = http_client_set_method(http_client, method);
    if(rc != 0) {
        vlogE("Express: Failed to set http method.(CURLE: %d)", rc);
        return ELA_EXPRESS_ERROR(conv_curlcode(rc));
    }

    if (method == HTTP_METHOD_POST) {
        http_client_set_header(http_client, "Content-Type", "application/binary");
        // http_client_set_header(http_client, "messageId", "10");

        rc = http_client_set_request_body(http_client, http_write_data, userdata);
        if (rc != 0) {
            vlogE("Express: Failed to set http request body.(CURLE: %d)", rc);
            return ELA_EXPRESS_ERROR(conv_curlcode(rc));
        }
    } else if (method == HTTP_METHOD_GET) {
        rc = http_client_set_response_body(http_client, http_read_data, userdata);
        if (rc != 0) {
            vlogE("Express: Failed to set http response body.(CURLE: %d)", rc);
            return ELA_EXPRESS_ERROR(conv_curlcode(rc));
        }
    }

    http_client_set_timeout(connector->http_client, EXP_HTTP_TIMEOUT);
    rc = http_client_request(http_client);
    if(rc != 0) {
        vlogE("Express: Failed to perform http request. url=%s,(CURLE: %d)", url, rc);
        return ELA_EXPRESS_ERROR(conv_curlcode(rc));
    }

    rc = http_client_get_response_code(http_client, &http_client_rescode);
    if(rc != 0) {
        vlogE("Express: Failed to get http response code.(CURLE: %d)", rc);
        return ELA_EXPRESS_ERROR(conv_curlcode(rc));
    }
    if((method == HTTP_METHOD_POST && http_client_rescode != 201)
    || (method == HTTP_METHOD_GET && http_client_rescode != 200)
    || (method == HTTP_METHOD_DELETE && http_client_rescode != 205)) {
        vlogE("Express: Failed to %s message from server rescode=%d.(CURLE: %d)", dowhat, http_client_rescode, rc);
        return ELA_EXPRESS_ERROR(ELAERR_WRONG_STATE);
    }

    vlogD("Express: Success to %s message, url: %s", dowhat, url);

    return 0;
}

static int del_msgs(ExpressConnector *connector, http_client_t *httpc,
                    int64_t msg_lasttime)
{
    int rc;
    char lasttime[256];
    int lasttime_len;
    uint8_t *crypted_data;
    int crypted_sz;
    char *encoded_data;
    size_t encoded_sz;
    char path[EXP_HTTP_URL_MAXSIZE];

    if(msg_lasttime <= 0)
        return 0;

    snprintf(lasttime, sizeof(lasttime), "%"PRId64, msg_lasttime);
    lasttime_len = strlen(lasttime);

    crypted_data = alloca(NONCE_BYTES + lasttime_len + ZERO_BYTES);
    rc = encrypt_data(connector->shared_key, (uint8_t*)lasttime, lasttime_len, crypted_data);
    if(rc < 0) {
        vlogE("Express: encrypt last tiime failed.(%x)", rc);
        return rc;
    }
    crypted_sz = rc;

    encoded_sz = crypted_sz * 1.4f + 1;
    encoded_data = alloca(encoded_sz);
    encoded_data = base58_encode(crypted_data, crypted_sz,
                                 encoded_data, &encoded_sz);
    if(encoded_data == NULL || encoded_sz <= 0) {
        vlogE("Express: encode last time failed.");
        return ELA_EXPRESS_ERROR(ELAERR_INVALID_CREDENTIAL);
    }

    snprintf(path, sizeof(path), "%s?until=%s", my_userid(connector), encoded_data);
    rc = http_do(connector, httpc,
                     path, HTTP_METHOD_DELETE,
                     NULL);
    if(rc < 0) {
        vlogE("Express: delete message failed.(%x)", rc);
        return rc;
    }

    return 0;
}

static int postmsg_runner(ExpTasklet *base)
{
    int rc;
    ExpSendTasklet *task = (ExpSendTasklet *)base;
    ExpressConnector *connector = task->base.connector;
    char path[EXP_HTTP_URL_MAXSIZE];

#if 0
    vlogV("Express: send message data: (%d) 0x%02x 0x%02x ~ 0x%02x 0x%02x",
          length, data[0], data[1], data[length - 2], data[length - 1]);
#endif

    snprintf(path, sizeof(path), "%s/%s", task->to, my_userid(connector));
    rc = http_do(connector, connector->http_client,
                     path, HTTP_METHOD_POST,
                     task);
    connector->on_stat_cb(connector->carrier, task->to, task->msgid, rc);
    if(rc < 0) {
        vlogE("Express: Failed to post message.(%x)", rc);
        return rc;
    }

    vlogD("Express: Success to post message to %s.", task->to);
    return 0;
}

static int pullmsgs_runner(ExpTasklet *base)
{
    int rc;
    ExpPullTasklet *task = (ExpPullTasklet *)base;
    ExpressConnector *connector = task->base.connector;
    const char *path;

    path = my_userid(connector);
    rc = http_do(connector, connector->http_client,
                     path, HTTP_METHOD_GET,
                     task);
    if(rc < 0) {
        vlogE("Express: Failed to post message.(%x)", rc);
        return rc;
    }
    vlogD("Express: Success to pull message from %s.", path);

    if(task->last_timestamp > 0) {
        rc = del_msgs(connector, connector->http_client, task->last_timestamp);
        if(rc < 0) {
            vlogE("Express: Failed to delete message.(%x)", rc);
            return rc;
        }
    }
    return 0;
}

static int enqueue_post_tasklet(ExpressConnector *connector, const char *to,
                                    const void *data, size_t size, int64_t msgid)
{
    int rc;
    ExpSendTasklet *task;
    uint8_t *crypted_data;
    ssize_t crypted_sz;

    assert(connector && to);

    crypted_sz = size + NONCE_BYTES + ZERO_BYTES;
    task = (ExpSendTasklet *)rc_zalloc(sizeof(ExpSendTasklet) + crypted_sz, NULL);
    if (!task) {
        rc = ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY);
        ela_set_error(rc);
        return rc;
    }

    task->base.entry.data = task;
    task->base.connector = connector;
    task->base.runner = postmsg_runner;

    strcpy(task->to, to);

    task->msgid = msgid;
    crypted_data = task->data;
    rc = encrypt_data(connector->shared_key, (uint8_t*)data, size, crypted_data);
    if(rc < 0) {
        vlogE("Express: Encrypt data error");
        deref(task);
        ela_set_error(rc);
        return rc;
    }
    crypted_sz = rc;

    task->pos = 0;
    task->length = crypted_sz;

    pthread_mutex_lock(&connector->lock);
    list_push_head(connector->task_list, &task->base.entry);
    pthread_mutex_unlock(&connector->lock);

    deref(task);
    pthread_cond_signal(&connector->cond);

    return 0;
}

static void connector_releaser(void *arg)
{
    ExpressConnector *connector = (ExpressConnector *)arg;
    assert(connector);

    if (connector->base_url) {
        free((void*)connector->base_url);
        connector->base_url = NULL;
    }

    if (connector->http_client) {
        http_client_close(connector->http_client);
        connector->http_client = NULL;
    }

    if (connector->task_list) {
        deref(connector->task_list);
        connector->task_list = NULL;
    }

    if (connector->carrier) {
        deref(connector->carrier);
        connector->carrier = NULL;
    }

    pthread_mutex_destroy(&connector->lock);
    pthread_cond_destroy (&connector->cond);
}

static void *connector_laundry(void *arg)
{
    int rc;
    ExpressConnector *connector = (ExpressConnector *)arg;
    ExpTasklet *task;

    vlogI("Express: Express connector start to running.");

    pthread_mutex_lock(&connector->lock);
    while(!connector->stop_flag) {
        if (list_is_empty(connector->task_list)) {
            pthread_cond_wait(&connector->cond, &connector->lock);
            continue;
        }

        task = list_pop_tail(connector->task_list);
        assert(task);

        pthread_mutex_unlock(&connector->lock);

        rc = task->runner(task);
        deref(task);
        // if(rc < 0) {
        //     vlogW("Express: exec tasklet failed.(%x)", rc);
        //     ela_set_error(rc);
        // }

        pthread_mutex_lock(&connector->lock);
    }
    pthread_mutex_unlock(&connector->lock);

    deref(connector);
    deref(connector->carrier);

    vlogI("Express: Express connector exited gracefully.");

    return NULL;
}

ExpressConnector *express_connector_create(ElaCarrier *carrier,
                                           ExpressOnRecvCallback on_msg_cb,
                                           ExpressOnRecvCallback on_req_cb,
                                           ExpressOnStatCallback on_stat_cb)
{
    int rc;
    ExpressConnector *connector;
    char url_base[EXP_HTTP_URL_MAXSIZE];
    const char* url_host;
    pthread_t tid;

    assert(carrier);

    connector = rc_zalloc(sizeof(ExpressConnector), connector_releaser); // deref by outside
    if (!connector) {
        ela_set_error(ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    connector->carrier = ref(carrier);
    connector->on_msg_cb = on_msg_cb;
    connector->on_req_cb = on_req_cb;
    connector->on_stat_cb = on_stat_cb;

    pthread_mutex_init(&connector->lock, NULL);
    pthread_cond_init (&connector->cond, NULL);

    connector->task_list = list_create(0, NULL);
    if (!connector->task_list) {
        deref(connector);
        ela_set_error(ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }
    connector->stop_flag = 0;

    connector->http_client = http_client_new();
    if (!connector->http_client) {
        deref(connector);
        ela_set_error(ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    if (carrier->pref.express_bootstraps_size < 0) {
        deref(connector);
        ela_set_error(ELA_EXPRESS_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    DhtBootstrapNodeBuf *express_bootstrap_0 = &carrier->pref.express_bootstraps[0];
    rc = compute_sharedkey(carrier, express_bootstrap_0->public_key, connector->shared_key);
    if (rc < 0) {
        deref(connector);
        ela_set_error(rc);
        return NULL;
    }
    url_host = (strlen(express_bootstrap_0->ipv4) != 0 ? express_bootstrap_0->ipv4 : express_bootstrap_0->ipv6); 
    snprintf(url_base, sizeof(url_base), "http://%s:%d", url_host, express_bootstrap_0->port);
    connector->base_url = strdup(url_base);
    connector->magic_num = ntohl(EXP_HTTP_MAGICNUM);

    rc = pthread_create(&tid, NULL, connector_laundry, connector);
    if (rc != 0) {
        deref(connector);
        ela_set_error(ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }
    pthread_detach(tid);

    // ref for connector_laundry thread,
    // and deref by connector_laundry
    ref(connector);

    return connector;
}

// peer with express_connector_create,
// connector must be deref manually after this function called.
void express_connector_kill(ExpressConnector *connector)
{
    assert(connector);

    pthread_mutex_lock(&connector->lock);
    connector->stop_flag = 1;
    pthread_cond_signal(&connector->cond);
    pthread_mutex_unlock(&connector->lock);
}

int express_enqueue_post_message(ExpressConnector *connector,
                                 const char *friendid,
                                 const void *data, size_t size)
{
    uint8_t shared_key[SYMMETRIC_KEY_BYTES];
    uint8_t *crypted_data;
    ssize_t crypted_sz;
    int rc;

    rc = compute_friend_sharedkey(connector->carrier, friendid, shared_key);
    if (rc < 0) {
        vlogE("Express: compute shared key error(%x)", rc);
        return rc;
    }

    crypted_data = (uint8_t *)calloc(1, size + NONCE_BYTES + ZERO_BYTES);
    if (!crypted_data) {
        vlogW("Express: post message failed.");
        return ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY);
    }
    rc = encrypt_data(shared_key, (uint8_t *)data, size, crypted_data);
    if(rc < 0) {
        free(crypted_data);
        vlogE("Express: encrypt last tiime failed.(%x)", rc);
        return rc;
    }
    crypted_sz = rc;

    rc = enqueue_post_tasklet(connector, friendid, crypted_data, crypted_sz, 0);
    free(crypted_data);
    if (rc < 0) {
        vlogE("Express: enqueue post tasklet error(%x)", rc);
        return rc;
    }

    return 0;
}

int express_enqueue_post_request(ExpressConnector *connector,
                                 const char *address,
                                 const void *hello, size_t size)
{
    return enqueue_post_tasklet(connector, address, hello, size, 0);
}

int express_enqueue_post_message_with_receipt(ExpressConnector *connector,
                                              const char *friendid,
                                              const void *data, size_t size,
                                              int64_t msgid)
{
    uint8_t shared_key[SYMMETRIC_KEY_BYTES];
    uint8_t *crypted_data;
    ssize_t crypted_sz;
    int rc;

    rc = compute_friend_sharedkey(connector->carrier, friendid, shared_key);
    if (rc < 0) {
        vlogE("Express: compute shared key error(%x)", rc);
        return rc;
    }

    crypted_data = (uint8_t *)calloc(1, size + NONCE_BYTES + ZERO_BYTES);
    if (!crypted_data) {
        vlogW("Express: post receipt message failed.");
        return ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY);
    }
    rc = encrypt_data(shared_key, (uint8_t *)data, size, crypted_data);
    if(rc < 0) {
        free(crypted_data);
        vlogE("Express: encrypt last tiime failed.(%x)", rc);
        return rc;
    }
    crypted_sz = rc;

    rc = enqueue_post_tasklet(connector, friendid, crypted_data, crypted_sz, msgid);
    free(crypted_data);
    if (rc < 0) {
        vlogE("Express: enqueue post tasklet error(%x)", rc);
        return rc;
    }

    return 0;
}

int express_enqueue_pull_messages(ExpressConnector *connector)
{
    ExpPullTasklet *task;

    assert(connector);

    task = (ExpPullTasklet *)rc_zalloc(sizeof(ExpPullTasklet), NULL);
    if (!task)
        return ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY);

    task->base.entry.data = task;
    task->base.connector = connector;
    task->base.runner = pullmsgs_runner;

    pthread_mutex_lock(&connector->lock);
    list_push_head(connector->task_list, &task->base.entry);
    pthread_mutex_unlock(&connector->lock);

    deref(task);
    pthread_cond_signal(&connector->cond);

    return 0;
}
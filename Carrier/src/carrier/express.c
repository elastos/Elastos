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

typedef struct ExpNode {
    char *ipv4;
    uint16_t port;
    uint8_t shared_key[DHT_PUBLIC_KEY_SIZE];
} ExpNode;

struct ExpConnector {
    ElaCarrier *carrier;
    ExpressOnRecvCallback on_msg_cb;
    ExpressOnRecvCallback on_req_cb;
    ExpressOnStatCallback on_stat_cb;

    pthread_mutex_t lock;
    pthread_cond_t cond;

    list_t *task_list;
    int stop_flag;

    uint32_t magic_num;

    http_client_t *http_client;
    int express_nodes_size;
    ExpNode express_nodes[0];
};

typedef struct ExpTasklet {
    list_entry_t entry;
    ExpressConnector *connector;

    int (*runner)(struct ExpTasklet *);
} ExpTasklet;

typedef struct {
    ExpTasklet base;
    struct {
        uint8_t *data;
        ssize_t size;
        size_t pos;
    } crypted_data_cache;

    char to[ELA_MAX_ADDRESS_LEN + 1];
    int64_t msgid;
    size_t data_size;
    uint8_t data[0];
} ExpSendTasklet;

typedef struct {
    ExpTasklet base;
    uint8_t *shared_key_cache;

    int64_t last_timestamp;
    size_t pos;
    uint8_t *data;
} ExpPullTasklet;

static const int  EXP_CURLCODE_MASK    = 0x00001000;
static const int  EXP_HTTP_MAGICNUM    = 0xCA6EE595;
static const int  EXP_HTTP_MAGICSIZE   = 4;
static const int  EXP_HTTP_REQ_TIMEOUT     = 30 * 1000; // ms
static const int  EXP_HTTP_HEAD_TIMEOUT     = 5 * 1000; // ms
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

static int parse_msg(ExpressConnector *connector, uint8_t *shared_key,
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

    rc = decrypt_data(shared_key,
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

static int parse_msg_stream(ExpressConnector *connector, uint8_t *shared_key,
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
            data_off = magic_off;
            break;
        }
        msg = &data[data_off];
        data_off += msg_sz;

        if(msg_sz == 0) { // empty message, ignore it
            continue;
        }
        rc = parse_msg(connector, shared_key, msg, msg_sz, timestamp);
        if(rc < 0) {
            vlogW("Express: response body parse message failed: (%x).",rc);
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

    rc = parse_msg_stream(connector, task->shared_key_cache, task->data, task->pos, &task->last_timestamp);
    if (rc < 0) {
        ela_set_error(rc);
    }
    parsed_sz = rc;

    if(parsed_sz >= 0 && parsed_sz < (int)task->pos) {
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
    int data_sz = task->crypted_data_cache.size - task->crypted_data_cache.pos;

    if(data_sz <= 0) {
        return 0;
    }

    data_sz = (data_sz < buf_sz ? data_sz : buf_sz);
    memcpy(buffer, task->crypted_data_cache.data + task->crypted_data_cache.pos, data_sz);

    task->crypted_data_cache.pos += data_sz;
    if(task->crypted_data_cache.pos >= task->crypted_data_cache.size) {
        task->crypted_data_cache.pos = 0;
        task->crypted_data_cache.size = 0;
    }

    return data_sz;
}

static int http_do(ExpressConnector *connector, http_client_t *http_client,
                   const char* ip, uint16_t port,
                   const char* path, http_method_t method,
                   void *userdata)
{
    int rc;
    char url[EXP_HTTP_URL_MAXSIZE];
    long http_client_rescode = 0;

    const char* dowhat = (method == HTTP_METHOD_HEAD ? "heading"
                       : (method == HTTP_METHOD_POST ? "pushing"
                       : (method == HTTP_METHOD_GET ? "pulling"
                       : (method == HTTP_METHOD_DELETE ? "deleting"
                       : "unknown"))));

    snprintf(url, sizeof(url), "https://%s:%d/%s", ip, port, path);
    // vlogD("Express: %s message, node: %s", dowhat, ip);

    http_client_reset(http_client);

    rc = http_client_set_url(http_client, url);
    if(rc != 0) {
        vlogE("Express: Failed to set url.(CURLE: %d)", rc);
        return ELA_EXPRESS_ERROR(conv_curlcode(rc));
    }

    rc = http_client_set_method(http_client, method);
    if(rc != 0) {
        vlogE("Express: Failed to set method.(CURLE: %d)", rc);
        return ELA_EXPRESS_ERROR(conv_curlcode(rc));
    }

    if (method == HTTP_METHOD_POST) {
        http_client_set_header(http_client, "Content-Type", "application/binary");
        // http_client_set_header(http_client, "messageId", "10");

        rc = http_client_set_request_body(http_client, http_write_data, userdata);
        if (rc != 0) {
            vlogE("Express: Failed to set request body.(CURLE: %d)", rc);
            return ELA_EXPRESS_ERROR(conv_curlcode(rc));
        }
    } else if (method == HTTP_METHOD_GET) {
        rc = http_client_set_response_body(http_client, http_read_data, userdata);
        if (rc != 0) {
            vlogE("Express: Failed to set response body.(CURLE: %d)", rc);
            return ELA_EXPRESS_ERROR(conv_curlcode(rc));
        }
    }

    http_client_set_timeout(connector->http_client,
                            method == HTTP_METHOD_HEAD ?  EXP_HTTP_HEAD_TIMEOUT : EXP_HTTP_REQ_TIMEOUT);
    rc = http_client_request(http_client);
    if(rc != 0) {
        vlogE("Express: Failed to perform request. node:%s, path:%s. (CURLE: %d)", ip, path, rc);
        return ELA_EXPRESS_ERROR(conv_curlcode(rc));
    }

    rc = http_client_get_response_code(http_client, &http_client_rescode);
    if(rc != 0) {
        vlogE("Express: Failed to get response code.(CURLE: %d)", rc);
        return ELA_EXPRESS_ERROR(conv_curlcode(rc));
    }
    if((method == HTTP_METHOD_POST && http_client_rescode != 201)
    || (method == HTTP_METHOD_GET && http_client_rescode != 200)
    || (method == HTTP_METHOD_DELETE && http_client_rescode != 205)) {
        vlogE("Express: Failed to %s message from node. rescode=%d.(CURLE: %d)", dowhat, http_client_rescode, rc);
        return ELA_EXPRESS_ERROR(ELAERR_WRONG_STATE);
    }

    // vlogD("Express: Success to %s message, node: %s", dowhat, ip);

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
    int idx;

    if(msg_lasttime <= 0)
        return 0;

    snprintf(lasttime, sizeof(lasttime), "%"PRId64, msg_lasttime);
    lasttime_len = strlen(lasttime);

    for(idx = 0; idx < connector->express_nodes_size; idx++) {
        crypted_data = alloca(NONCE_BYTES + lasttime_len + ZERO_BYTES);
        rc = encrypt_data(connector->express_nodes[idx].shared_key, (uint8_t*)lasttime, lasttime_len, crypted_data);
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
                     connector->express_nodes[idx].ipv4, connector->express_nodes[idx].port,
                     path, HTTP_METHOD_DELETE,
                     NULL);
        if (rc >= 0)
            break;
    }
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
    int idx;

#if 0
    vlogV("Express: send message data: (%d) 0x%02x 0x%02x ~ 0x%02x 0x%02x",
          length, data[0], data[1], data[length - 2], data[length - 1]);
#endif

    snprintf(path, sizeof(path), "%s/%s", task->to, my_userid(connector));
    for(idx = 0; idx < connector->express_nodes_size; idx++) {
        task->crypted_data_cache.pos = 0;
        task->crypted_data_cache.size = task->data_size + NONCE_BYTES + ZERO_BYTES;
        task->crypted_data_cache.data = calloc(1, task->crypted_data_cache.size);
        if (!task->crypted_data_cache.data) {
            vlogE("Express: alloc encrypt data error");
            deref(task);
            ela_set_error(rc);
            return rc;
        }

        rc = encrypt_data(connector->express_nodes[idx].shared_key, (uint8_t *)task->data, task->data_size,
                          task->crypted_data_cache.data);
        if (rc < 0) {
            vlogE("Express: Encrypt data error");
            deref(task);
            ela_set_error(rc);
            return rc;
        }
        task->crypted_data_cache.size = rc;

        rc = http_do(connector, connector->http_client,
                     connector->express_nodes[idx].ipv4, connector->express_nodes[idx].port,
                     path, HTTP_METHOD_POST,
                     task);
        free((void*)task->crypted_data_cache.data);
        task->crypted_data_cache.pos = -1;
        task->crypted_data_cache.size = -1;
        task->crypted_data_cache.data = NULL;
        if (rc >= 0)
            break;
    }

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
    int idx;

    path = my_userid(connector);
    for(idx = 0; idx < connector->express_nodes_size; idx++) {
        task->shared_key_cache = connector->express_nodes[idx].shared_key;
        rc = http_do(connector, connector->http_client,
                     connector->express_nodes[idx].ipv4, connector->express_nodes[idx].port,
                     path, HTTP_METHOD_GET,
                     task);
        if (rc >= 0)
            break;
    }
    if(rc < 0) {
        vlogE("Express: Failed to pull message.(%x)", rc);
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

static int speedmeter_runner(ExpTasklet *base)
{
    int rc;
    ExpTasklet *task = (ExpTasklet *)base;
    ExpressConnector *connector = task->connector;
    int idx, iidx, jidx;
    struct timeval starttime;
    struct timeval *timeloss;
    
    timeloss = alloca(sizeof(*timeloss) * connector->express_nodes_size);
    memset(timeloss, 0, sizeof(*timeloss) * connector->express_nodes_size);

    for(idx = 0; idx < connector->express_nodes_size; idx++) {
        gettimeofday(&starttime, NULL);
        rc = http_do(connector, connector->http_client,
                     connector->express_nodes[idx].ipv4, connector->express_nodes[idx].port,
                     "version", HTTP_METHOD_HEAD, NULL);
        if(rc >= 0) {
            gettimeofday(&timeloss[idx], NULL);
            timersub(&timeloss[idx], &starttime, &timeloss[idx]);
        } else {
            timeloss[idx].tv_sec = EXP_HTTP_HEAD_TIMEOUT;
        }
    }

    // bubble sort
    int total_count = connector->express_nodes_size - 1;
    for (iidx = 0; iidx < total_count; iidx++) {
        for (jidx = 0; jidx < total_count - iidx; jidx++) {
            if (timercmp(&timeloss[jidx], &timeloss[jidx + 1], >)) {
                struct timeval tmptime;
                ExpNode tmpnode;

                memcpy(&tmptime, &timeloss[jidx + 1], sizeof(struct timeval));
                memcpy(&timeloss[jidx + 1], &timeloss[jidx], sizeof(struct timeval));
                memcpy(&timeloss[jidx], &tmptime, sizeof(struct timeval));

                memcpy(&tmpnode, &connector->express_nodes[jidx + 1], sizeof(ExpNode));
                memcpy(&connector->express_nodes[jidx + 1], &connector->express_nodes[jidx], sizeof(ExpNode));
                memcpy(&connector->express_nodes[jidx], &tmpnode, sizeof(ExpNode));
            }
        }
    }

    vlogD("Express: meter express node speed:");
    for(idx = 0; idx < connector->express_nodes_size; idx++) {
        vlogD("  %s ==> %ld%06ld",
              connector->express_nodes[idx].ipv4,
              timeloss[idx].tv_sec, timeloss[idx].tv_usec);
    }

    return 0;
}

static int enqueue_speedmeter_tasklet(ExpressConnector *connector)
{
    int rc;
    ExpTasklet *task;

    assert(connector);

    task = (ExpTasklet *)rc_zalloc(sizeof(ExpTasklet), NULL);
    if (!task) {
        rc = ELA_EXPRESS_ERROR(ELAERR_OUT_OF_MEMORY);
        ela_set_error(rc);
        return rc;
    }

    task->entry.data = task;
    task->connector = connector;
    task->runner = speedmeter_runner;

    pthread_mutex_lock(&connector->lock);
    list_push_head(connector->task_list, &task->entry);
    pthread_mutex_unlock(&connector->lock);

    deref(task);
    pthread_cond_signal(&connector->cond);

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

    task = (ExpSendTasklet *)rc_zalloc(sizeof(ExpSendTasklet) + size, NULL);
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
    memcpy(task->data, data, size);
    task->data_size = size;

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
    ExpressNodeBuf *node_0;
    pthread_t tid;
    int idx;

    assert(carrier);
    if (carrier->pref.express_nodes_size <= 0) {
        ela_set_error(ELA_EXPRESS_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    connector = rc_zalloc(sizeof(ExpressConnector) + sizeof(ExpNode) * carrier->pref.express_nodes_size,
                          connector_releaser); // deref by outside
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

    connector->magic_num = ntohl(EXP_HTTP_MAGICNUM);

    connector->express_nodes_size = carrier->pref.express_nodes_size;
    for(idx = 0; idx < connector->express_nodes_size; idx++) {
        connector->express_nodes[idx].ipv4 = carrier->pref.express_nodes[idx].ipv4;
        connector->express_nodes[idx].port = carrier->pref.express_nodes[idx].port;
        rc = compute_sharedkey(carrier,
                               carrier->pref.express_nodes[idx].public_key,
                               connector->express_nodes[idx].shared_key);
        if (rc < 0) {
            deref(connector);
            ela_set_error(rc);
            return NULL;
        }
    }

    rc = enqueue_speedmeter_tasklet(connector);
    if (rc < 0) {
        deref(connector);
        ela_set_error(rc);
        return NULL;
    }

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
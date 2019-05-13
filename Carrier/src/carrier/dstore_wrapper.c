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
#include <string.h>
#include <limits.h>
#include <pthread.h>

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
#endif

#include <crystal.h>
#include <hive/c-api.h>

#include "dht.h"
#include "ela_carrier.h"
#include "ela_carrier_impl.h"
#include "dstore_wrapper.h"

struct DStoreWrapper {
    ElaCarrier *carrier;
    DStoreOnMsgCallback cb;
    DStoreC *dstore;
    list_t *msg_list;

    pthread_mutex_t lock;
    pthread_cond_t cond;
    int stopped;
};

typedef struct DStoreMsg DStoreMsg;
struct DStoreMsg {
    DStoreWrapper *ctx;
    list_entry_t le;
    void (*handle_cb)(DStoreMsg *);
};

typedef struct DStorePollMsg {
    DStoreMsg base;
} DStorePollMsg;

typedef struct DStoreOffMsg {
    DStoreMsg base;
    char key[(SHA256_BYTES << 1) + 4];
    size_t val_sz;
    char val[4];
} DStoreOffMsg;

static const char *dstore_data_filename = "dstore.data";

static inline uint8_t *compute_nonce(const char *dstore_key)
{
    uint8_t offset;

    offset = dstore_key[0] % ((SHA256_BYTES << 1) - NONCE_BYTES + 1);
    return (uint8_t *)dstore_key + offset;
}

static void dstore_offmsg_send(DStoreMsg *base)
{
    DStoreOffMsg  *msg = (DStoreOffMsg *)base;
    DStoreWrapper *ctx = base->ctx;
    int rc;

    assert(msg);
    assert(ctx);
    assert(ctx->dstore);

    rc = dstore_add_value(ctx->dstore, msg->key, (const uint8_t *)msg->val,
                          msg->val_sz);
    if (rc < 0)
        vlogE("Carrier: Add Key-Value to dstore error.(0x%x)", rc);
}

int dstore_enqueue_offmsg(DStoreWrapper *ctx, const char *friendid,
                           const void *msg, size_t length)
{
    uint8_t self_sk[SECRET_KEY_BYTES];
    uint8_t self_pk[PUBLIC_KEY_BYTES];
    uint8_t peer_pk[PUBLIC_KEY_BYTES];
    uint8_t sharedkey[SYMMETRIC_KEY_BYTES];
    uint8_t *msgbody = alloca(MAC_BYTES + length);
    ssize_t size;
    uint8_t *nonce;
    char msgkey[(SHA256_BYTES << 1) + 1];
    char *key;
    DStoreOffMsg *offmsg;

    size = base58_decode(friendid, strlen(friendid), peer_pk, sizeof(peer_pk));
    if ((size_t)size != sizeof(peer_pk))  {
        vlogE("Carrier: Decode base58 friendid %s error", friendid);
        return ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS);
    }

    dht_self_get_secret_key(&ctx->carrier->dht, self_sk);
    dht_self_get_public_key(&ctx->carrier->dht, self_pk);
    crypto_compute_symmetric_key(peer_pk, self_sk, sharedkey);

    // Compute sending msgkey.
    // msgkey=SHA256<SYMMTRIC(self_sk, peer_pk), peer_pk>
    key = hmac_sha256a(sharedkey, SYMMETRIC_KEY_BYTES,
                       peer_pk, PUBLIC_KEY_BYTES,
                       msgkey, sizeof(msgkey));
    if (!key) {
        vlogE("Carrier: Computing sending message key error.");
        return ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS);
    }

    nonce = compute_nonce(msgkey);
    size  = crypto_encrypt(sharedkey, nonce, msg, length, msgbody);
    if (size < 0)
        return ELA_GENERAL_ERROR(ELAERR_ENCRYPT);

    offmsg = (DStoreOffMsg *)rc_zalloc(sizeof(DStoreOffMsg) + size, NULL);
    if (!offmsg)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    offmsg->base.ctx = ctx;
    offmsg->base.handle_cb = dstore_offmsg_send;
    offmsg->base.le.data = offmsg;
    offmsg->val_sz = (size_t)size;
    strcpy(offmsg->key, msgkey);
    memcpy(offmsg->val, msgbody, size);

    pthread_mutex_lock(&ctx->lock);
    list_push_head(ctx->msg_list, &offmsg->base.le);
    pthread_mutex_unlock(&ctx->lock);

    deref(offmsg);
    pthread_cond_signal(&ctx->cond);

    return 0;
}

static bool get_offmsg_body(const char *msg_key,
                            const uint8_t *buf, size_t length,
                            void *context)
{
    DStoreWrapper *ctx = (DStoreWrapper *)((void **)context)[0];
    uint8_t *friendkey = (uint8_t *)((void **)context)[1];
    uint8_t *sharedkey  = (uint8_t *)((void **)context)[2];

    uint8_t *msg = alloca(length - MAC_BYTES);
    ssize_t len;
    const uint8_t *nonce;
    char friendid[ELA_MAX_ID_LEN + 1] = {0};
    size_t idlen = sizeof(friendid);

    nonce = compute_nonce(msg_key);
    len  = crypto_decrypt(sharedkey, nonce, buf, length, msg);
    if (len <= 0) {
        vlogE("Carrier: decrypt offline message body error.");
        return false;
    }

    base58_encode(friendkey, DHT_PUBLIC_KEY_SIZE, friendid, &idlen);
    ctx->cb(ctx->carrier, friendid, msg, len);
    return true;
}

static bool check_offmsg_cb(uint32_t friend_number, const uint8_t *friend_pk,
                            int user_status,
                            const uint8_t *desc, size_t desc_length,
                            void *context)
{
    DStoreWrapper *ctx = (DStoreWrapper *)context;
    ElaCarrier *w = ctx->carrier;
    uint8_t self_sk[SECRET_KEY_BYTES];
    uint8_t self_pk[PUBLIC_KEY_BYTES];
    uint8_t sharedkey[SYMMETRIC_KEY_BYTES];
    char msgkey[(SHA256_BYTES << 1) + 1];
    char *key;
    void *argv[] = {
        ctx,
        (void *)friend_pk,
        sharedkey,
    };

    (void)friend_number;
    (void)user_status;
    (void)desc;
    (void)desc_length;

    dht_self_get_secret_key(&w->dht, self_sk);
    dht_self_get_public_key(&w->dht, self_pk);
    crypto_compute_symmetric_key(friend_pk, self_sk, sharedkey);

    // Compute receiving msgkey.
    // msgkey=SHA256<SYMMETRIC(self_sk, peer_pk), self_pk>
    key = hmac_sha256a(sharedkey, SYMMETRIC_KEY_BYTES,
                       self_pk, PUBLIC_KEY_BYTES,
                       msgkey, sizeof(msgkey));
    if (!key) {
        vlogE("Carrier: Computing receiving msgkey error.");
        return true;
    }

    // iterate messages from friend with friend_pk.
    dstore_get_values(ctx->dstore, key, &get_offmsg_body, argv);
    dstore_remove_values(ctx->dstore, key);

    return true;
}

static void dstore_pollmsg_callback(DStoreMsg *base)
{
    DStoreWrapper *ctx = base->ctx;
    dht_get_friends(&ctx->carrier->dht, check_offmsg_cb, ctx);
}

void dstore_enqueue_pollmsg(DStoreWrapper *ctx)
{
    DStorePollMsg *pollmsg;

    assert(ctx);

    pollmsg = (DStorePollMsg *)rc_zalloc(sizeof(DStorePollMsg), NULL);
    if (!pollmsg)
        return;

    pollmsg->base.ctx = ctx;
    pollmsg->base.handle_cb = dstore_pollmsg_callback;
    pollmsg->base.le.data = pollmsg;

    pthread_mutex_lock(&ctx->lock);
    list_push_head(ctx->msg_list, &pollmsg->base.le);
    pthread_mutex_unlock(&ctx->lock);

    deref(pollmsg);
    pthread_cond_signal(&ctx->cond);
}

static int redeem_dstore_file(ElaCarrier *w, char *data_path, size_t length)
{
    const char *conf_str = NULL;
    FILE *fp;
    size_t nwr;
    int rc;
    int i;

    assert(data_path);

    rc = snprintf(data_path, length, "%s/%s", w->pref.data_location,
                  dstore_data_filename);
    if (rc < 0 || rc >= (int)length)
        return -1;

    if (!access(data_path, F_OK))
        return 0;

    for (i = 0; i < w->pref.hive_bootstraps_size && !conf_str; ++i) {
        if (w->pref.hive_bootstraps[i].ipv4[0])
            conf_str = hive_generate_conf(w->pref.hive_bootstraps[i].ipv4,
                                          w->pref.hive_bootstraps[i].port);

        if (!conf_str && w->pref.hive_bootstraps[i].ipv6[0])
            conf_str = hive_generate_conf(w->pref.hive_bootstraps[i].ipv6,
                                          w->pref.hive_bootstraps[i].port);
    }

    if (!conf_str) {
        vlogE("Carrier: Generating dstore bootstrap seeds error");
        return -1;
    }

    fp = fopen(data_path, "w");
    if (!fp) {
        free((void *)conf_str);
        return -1;
    }

    nwr = fwrite(conf_str, strlen(conf_str), 1, fp);
    fclose(fp);
    free((void *)conf_str);

    return (nwr != 1) ? -1 : 0;
}

static void *dstore_msgs_dispatch(void *arg)
{
    DStoreWrapper *ctx = (DStoreWrapper *)arg;
    ElaCarrier *w = ctx->carrier;
    char conf_path[PATH_MAX];
    DStoreMsg *msg;
    int rc;

    rc = redeem_dstore_file(w, conf_path, sizeof(conf_path));
    if (rc < 0) {
        vlogE("Carrier: Redeem hive bootstraps nodes");
        deref(ctx);
        deref(ctx->carrier);
        return NULL;
    }

    ctx->dstore = dstore_create(conf_path);
    if (!ctx->dstore) {
        vlogE("Carrier: Create Dstore object error");
        deref(ctx);
        deref(ctx->carrier);
        return NULL;
    }
    vlogI("Carrier: Dstore is ready now.");

    dstore_enqueue_pollmsg(ctx); // First time to poll receiving messages.

    pthread_mutex_lock(&ctx->lock);
    while(!ctx->stopped) {
        if (list_is_empty(ctx->msg_list)) {
            pthread_cond_wait(&ctx->cond, &ctx->lock);
            continue;
        }

        msg = list_pop_tail(ctx->msg_list);
        assert(msg);

        pthread_mutex_unlock(&ctx->lock);

        msg->handle_cb(msg);
        deref(msg);

        pthread_mutex_lock(&ctx->lock);
    }
    pthread_mutex_unlock(&ctx->lock);

    deref(ctx);
    deref(ctx->carrier);

    return NULL;
}

static void DStoreWrapperDestroy(void *arg)
{
    DStoreWrapper *ctx = (DStoreWrapper *)arg;

    assert(ctx);

    if (ctx->carrier)
        ctx->carrier = NULL;

    if (ctx->dstore) {
        dstore_destroy(ctx->dstore);
        ctx->dstore = NULL;
    }

    if (ctx->msg_list) {
        deref(ctx->msg_list);
        ctx->msg_list = NULL;
    }

    pthread_mutex_destroy(&ctx->lock);
    pthread_cond_destroy(&ctx->cond);
}

DStoreWrapper *dstore_wrapper_create(ElaCarrier *w, DStoreOnMsgCallback cb)
{
    DStoreWrapper *ctx;
    pthread_t tid;
    int rc;

    ctx = rc_zalloc(sizeof(DStoreWrapper), DStoreWrapperDestroy);
    if (!ctx) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    pthread_mutex_init(&ctx->lock, NULL);
    pthread_cond_init(&ctx->cond, NULL);

    ctx->msg_list = list_create(0, NULL);
    if (!ctx->msg_list) {
        deref(ctx);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    ctx->carrier = w;
    ctx->cb = cb;
    ctx->stopped = 0;

    ref(w);
    ref(ctx);
    rc = pthread_create(&tid, NULL, dstore_msgs_dispatch, ctx);
    if (rc != 0) {
        deref(ctx);
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    pthread_detach(tid);
    return ctx;
}

void dstore_wrapper_kill(DStoreWrapper *ctx)
{
    assert(ctx);

    pthread_mutex_lock(&ctx->lock);
    ctx->stopped = 1;
    pthread_cond_signal(&ctx->cond);
    pthread_mutex_unlock(&ctx->lock);
}

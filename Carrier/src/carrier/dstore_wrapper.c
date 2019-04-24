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

enum {
    DSTORE_STATE_CRAWL,
    DSTORE_STATE_IDLE,
    DSTORE_STATE_STOP
};

struct DStoreWrapper {
    ElaCarrier *carrier;
    DStoreOnMsgCallback cb;
    DStoreC *dstore;
    int state;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

static inline uint8_t *compute_nonce(const char *dstore_key)
{
    uint8_t offset;

    offset = dstore_key[0] % ((SHA256_BYTES << 1) - NONCE_BYTES + 1);
    return (uint8_t *)dstore_key + offset;
}

ssize_t dstore_send_msg(DStoreWrapper *ctx, const char *friendid,
                      const void *msg, size_t length)
{
    uint8_t self_sk[SECRET_KEY_BYTES];
    uint8_t self_pk[PUBLIC_KEY_BYTES];
    uint8_t peer_pk[PUBLIC_KEY_BYTES];
    uint8_t sharedkey[SYMMETRIC_KEY_BYTES];
    uint8_t *msgbody = alloca(MAC_BYTES + length);
    ssize_t size;
    ssize_t len;
    uint8_t *nonce;
    char msgkey[(SHA256_BYTES << 1) + 1];
    char *key;
    int rc;

    if (!ctx->dstore) {
        vlogE("Carrier: DStore is not ready.");
        return ELA_GENERAL_ERROR(ELAERR_WRONG_STATE);
    }

    len = base58_decode(friendid, strlen(friendid), peer_pk, sizeof(peer_pk));
    if (len != sizeof(peer_pk))  {
        vlogE("Carrier: Decode friendid %s error.", friendid);
        return ELA_GENERAL_ERROR(ELAERR_ENCRYPT);
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
        vlogE("Carrier: Computing sending msgkey error.");
        return ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS);
    }

    nonce = compute_nonce(msgkey);
    size  = crypto_encrypt(sharedkey, nonce, msg, length, msgbody);
    if (size < 0) {
        vlogE("Carrier: Encrypt offline message body error.");
        return ELA_GENERAL_ERROR(ELAERR_ENCRYPT);
    }

    rc = dstore_add_value(ctx->dstore, msgkey, msgbody, size);
    if (rc < 0) {
        vlogE("Carrier: Sending offline message <K,V> error.");
        return ELA_GENERAL_ERROR(ELAERR_BUSY);
    }

    return length;
}

static bool get_msg_body(const char *msg_key,
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

static bool check_offline_msg_cb(uint32_t friend_number,
                                 const uint8_t *friend_pk,
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
        return false;
    }

    // iterate messages from friend with friend_pk.
    dstore_get_values(ctx->dstore, key, &get_msg_body, argv);
    dstore_remove_values(ctx->dstore, key);

    return true;
}

static int redeem_dstore_file(ElaCarrier *w,
                             char *conf_path, size_t length,
                             DStoreWrapper *ctxt)
{
    FILE *fp;
    size_t nwr;
    const char *conf_str = NULL;
    int i;
    int rc;

    rc = snprintf(conf_path, length, "%s/dstore_cache.conf", w->pref.data_location);
    if (rc < 0 || rc >= (int)length)
        return -1;

    if (!access(conf_path, F_OK))
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

    fp = fopen(conf_path, "w");
    if (!fp)
        return -1;

    nwr = fwrite(conf_str, strlen(conf_str), 1, fp);
    fclose(fp);
    free((void *)conf_str);
    return (nwr != 1) ? -1 : 0;
}

static void *crawl_offline_msg(void *arg)
{
    DStoreWrapper *ctx = (DStoreWrapper *)arg;
    ElaCarrier *w = ctx->carrier;
    int rc;
    char conf_path[PATH_MAX];

    rc = redeem_dstore_file(w, conf_path, sizeof(conf_path), ctx);
    if (rc < 0) {
        pthread_mutex_lock(&ctx->lock);
        ctx->state = DSTORE_STATE_STOP;
        pthread_mutex_unlock(&ctx->lock);
        deref(ctx);
        return NULL;
    }

    ctx->dstore = dstore_create(conf_path);
    if (!ctx->dstore) {
        pthread_mutex_lock(&ctx->lock);
        ctx->state = DSTORE_STATE_STOP;
        pthread_mutex_unlock(&ctx->lock);
        deref(ctx);
        return NULL;
    }

    while (true) {
        pthread_mutex_lock(&ctx->lock);
        while (ctx->state == DSTORE_STATE_IDLE)
            pthread_cond_wait(&ctx->cond, &ctx->lock);
        if (ctx->state == DSTORE_STATE_STOP) {
            pthread_mutex_unlock(&ctx->lock);
            deref(ctx);
            return NULL;
        }

        ctx->state = DSTORE_STATE_IDLE;
        pthread_mutex_unlock(&ctx->lock);
        dht_get_friends(&ctx->carrier->dht, check_offline_msg_cb, ctx);
    }

    return NULL;
}

static void DStoreWrapperDestroy(void *arg)
{
    DStoreWrapper *ctx = (DStoreWrapper *)arg;

    if (ctx->dstore) {
        dstore_destroy(ctx->dstore);
        ctx->dstore = NULL;
    }
    pthread_mutex_destroy(&ctx->lock);
    pthread_cond_destroy(&ctx->cond);
}

DStoreWrapper *dstore_wrapper_create(ElaCarrier *w, DStoreOnMsgCallback cb)
{
    DStoreWrapper *ctx;
    int rc;
    pthread_t worker;

    ctx = rc_zalloc(sizeof(DStoreWrapper), DStoreWrapperDestroy);
    if (!ctx)
        return NULL;

    pthread_mutex_init(&ctx->lock, NULL);
    pthread_cond_init(&ctx->cond, NULL);

    ctx->carrier = w;
    ctx->cb = cb;

    ref(ctx);
    rc = pthread_create(&worker, NULL, crawl_offline_msg, ctx);
    if (rc != 0) {
        deref(ctx);
        return NULL;
    }
    pthread_detach(worker);

    return ctx;
}

void dstore_wrapper_destroy(DStoreWrapper *ctx)
{
    pthread_mutex_lock(&ctx->lock);
    if (ctx->state == DSTORE_STATE_STOP) {
        pthread_mutex_unlock(&ctx->lock);
        return;
    }
    ctx->state = DSTORE_STATE_STOP;
    pthread_mutex_unlock(&ctx->lock);
    pthread_cond_signal(&ctx->cond);
    deref(ctx);
}

void notify_crawl_offline_msg(DStoreWrapper *ctx)
{
    pthread_mutex_lock(&ctx->lock);
    if (ctx->state == DSTORE_STATE_IDLE)
        ctx->state = DSTORE_STATE_CRAWL;
    pthread_mutex_unlock(&ctx->lock);
    pthread_cond_signal(&ctx->cond);
}

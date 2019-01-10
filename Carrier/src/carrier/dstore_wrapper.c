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
#include <string.h>
#include <vlog.h>
#include <crypto.h>
#include <base58.h>
#include <pthread.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <rc_mem.h>
#include <hive/c-api.h>
#include <stdio.h>
#include <limits.h>
#if defined(_WIN32) || defined(_WIN64)
    #ifdef HAVE_MALLOC_H
    #include <malloc.h>
    #endif
    #include <io.h>
    #define PATH_MAX _MAX_PATH
    #define access _access
    #define F_OK (0)
#else
    #ifdef HAVE_ALLOCA_H
    #include <alloca.h>
    #endif
#endif
#include "dht.h"
#include "ela_carrier.h"
#include "ela_carrier_impl.h"
#include "dstore_wrapper.h"

struct DStoreWrapper {
    ElaCarrier *carrier;
    DStoreOnMsgCallback cb;
    pthread_t worker;
    DStoreC *dstore;
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

static void * crawl_offline_msg(void *arg)
{
    DStoreWrapper *ctx = (DStoreWrapper *)arg;
    dht_get_friends(&ctx->carrier->dht, check_offline_msg_cb, ctx);
    return NULL;
}

static void DStoreWrapperDestroy(void *arg)
{
    DStoreWrapper *ctx = (DStoreWrapper *)arg;

    if (ctx->dstore) {
        dstore_destroy(ctx->dstore);
        ctx->dstore = NULL;
    }
}

DStoreWrapper *dstore_wrapper_create(ElaCarrier *w, DStoreOnMsgCallback cb)
{
    DStoreWrapper *ctx;
    int rc;
    char conf_cache[PATH_MAX];

    ctx = rc_zalloc(sizeof(DStoreWrapper), DStoreWrapperDestroy);
    if (!ctx)
        return NULL;

    rc = snprintf(conf_cache, sizeof(conf_cache), "%s/dstore_cache.conf",
                  w->pref.data_location);
    if (rc >= sizeof(conf_cache)) {
        deref(ctx);
        return NULL;
    }

    if (access(conf_cache, F_OK)) {
        FILE *fp = fopen(conf_cache, "w");
        if (!fp) {
            deref(ctx);
            return NULL;
        }

        const char *conf_str = NULL;
        int i;
        for (i = 0; i < w->pref.hive_bootstraps_size && !conf_str; ++i) {
            if (w->pref.hive_bootstraps[i].ipv4[0]) {
                conf_str = hive_generate_conf(w->pref.hive_bootstraps[i].ipv4,
                                              w->pref.hive_bootstraps[i].port);
            }
            if (!conf_str && w->pref.hive_bootstraps[i].ipv6[0]) {
                conf_str = hive_generate_conf(w->pref.hive_bootstraps[i].ipv6,
                                              w->pref.hive_bootstraps[i].port);
            }
        }
        if (!conf_str) {
            deref(ctx);
            fclose(fp);
            remove(conf_cache);
            return NULL;
        }

        size_t nwr = fwrite(conf_str, strlen(conf_str), 1, fp);
        fclose(fp);
        free((void *)conf_str);
        if (nwr != 1) {
            remove(conf_cache);
            deref(ctx);
            return NULL;
        }
    }

    ctx->dstore = dstore_create(conf_cache);
    if (!ctx->dstore) {
        deref(ctx);
        return NULL;
    }

    ctx->carrier = w;
    ctx->cb = cb;

    rc = pthread_create(&ctx->worker, NULL, crawl_offline_msg, ctx);
    if (rc != 0) {
        deref(ctx);
        return NULL;
    }

    return ctx;
}

void dstore_wrapper_destroy(DStoreWrapper *ctx)
{
    // worker should be started, so join for exiting.
    pthread_join(ctx->worker, NULL);
    deref(ctx);
}

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
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <rc_mem.h>
#include <base58.h>
#include <vlog.h>
#include <crypto.h>

#include "ela_carrier.h"
#include "ela_carrier_impl.h"
#include "ela_turnserver.h"
#include "friends.h"
#include "tcallbacks.h"
#include "thistory.h"
#include "elacp.h"
#include "dht.h"

#define TURN_SERVER_PORT                ((uint16_t)3478)
#define TURN_SERVER_USER_SUFFIX         "auth.tox"
#define TURN_REALM                      "elastos.org"

#if defined(__ANDROID__)
extern int PJ_JNI_OnLoad(void *vm, void* reserved);

bool ela_android_onload(void *vm, void *reserved)
{
    int rc;

    rc = PJ_JNI_OnLoad(vm, reserved);
    return (rc >= 0);
}
#endif

const char* ela_get_version(void)
{
    return "elacarrier-5.0.1";
}

static bool is_valid_key(const char *key)
{
    char result[DHT_PUBLIC_KEY_SIZE << 1];
    ssize_t len;

    len = base58_decode(key, strlen(key), result, sizeof(result));
    return len == DHT_PUBLIC_KEY_SIZE;
}

bool ela_id_is_valid(const char *id)
{
    if (!id || !*id) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return false;
    }

    return is_valid_key(id);
}

static uint16_t address_checksum(const uint8_t *address, uint32_t len)
{
    uint8_t checksum[2] = {0};
    uint16_t check;
    uint32_t i;

    for (i = 0; i < len; ++i)
        checksum[i % 2] ^= address[i];

    memcpy(&check, checksum, sizeof(check));
    return check;
}

static bool is_valid_address(const char *address)
{
    uint8_t addr[DHT_ADDRESS_SIZE];
    uint16_t check, checksum;
    ssize_t len;

    len = base58_decode(address, strlen(address), addr, sizeof(addr));
    if (len != DHT_ADDRESS_SIZE)
        return false;

    memcpy(&check, addr + DHT_PUBLIC_KEY_SIZE + sizeof(uint32_t), sizeof(check));
    checksum = address_checksum(addr, DHT_ADDRESS_SIZE - sizeof(checksum));

    return checksum == check;
}

bool ela_address_is_valid(const char *address)
{
    if (!address || !*address) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return false;
    }

    return is_valid_address(address);
}

char *ela_get_id_by_address(const char *address, char *userid, size_t len)
{
    uint8_t addr[DHT_ADDRESS_SIZE];
    ssize_t addr_len;
    char *ret_userid;
    size_t userid_len = ELA_MAX_ID_LEN + 1;

    if (len <= ELA_MAX_ID_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    addr_len = base58_decode(address, strlen(address), addr, sizeof(addr));
    if (addr_len != DHT_ADDRESS_SIZE) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    memset(userid, 0, len);
    ret_userid = base58_encode(addr, DHT_PUBLIC_KEY_SIZE, userid, &userid_len);
    if (ret_userid == NULL) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    return ret_userid;
}

void ela_log_init(ElaLogLevel level, const char *log_file,
                  void (*log_printer)(const char *format, va_list args))
{
#if !defined(__ANDROID__)
    vlog_init(level, log_file, log_printer);
#endif
}

static
int get_friend_number(ElaCarrier *w, const char *friendid, uint32_t *friend_number)
{
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
    ssize_t len;
    int rc;

    assert(w);
    assert(friendid);
    assert(friend_number);

    len = base58_decode(friendid, strlen(friendid), public_key, sizeof(public_key));
    if (len != DHT_PUBLIC_KEY_SIZE) {
        vlogE("Carrier: friendid %s not base58 encoded.", friendid);
        return ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS);
    }

    rc = dht_get_friend_number(&w->dht, public_key, friend_number);
    if (rc < 0) {
        //vlogE("Carrier: friendid %s is not friend yet.", friendid);
        return ELA_GENERAL_ERROR(ELAERR_NOT_EXIST);
    }

    return rc;
}

static void fill_empty_user_desc(ElaCarrier *w)
{
    ElaCP *cp;
    uint8_t *data;
    size_t data_len;

    assert(w);

    cp = elacp_create(ELACP_TYPE_USERINFO, NULL);
    if (!cp) {
        vlogE("Carrier: Out of memory!!!");
        return;
    }

    elacp_set_has_avatar(cp, false);
    elacp_set_name(cp, "");
    elacp_set_descr(cp, "");
    elacp_set_gender(cp, "");
    elacp_set_phone(cp, "");
    elacp_set_email(cp, "");
    elacp_set_region(cp, "");

    data = elacp_encode(cp, &data_len);
    elacp_free(cp);

    if (!data) {
        vlogE("Carrier: Encode user desc to packet error");
        return;
    }

    dht_self_set_desc(&w->dht, data, data_len);
    free(data);
}

static
int unpack_user_desc(const uint8_t *desc, size_t desc_len, ElaUserInfo *info,
                     bool *changed)
{
    ElaCP *cp;
    const char *name;
    const char *descr;
    const char *gender;
    const char *phone;
    const char *email;
    const char *region;
    bool has_avatar;
    bool did_changed = false;

    assert(desc);
    assert(desc_len > 0);
    assert(info);

    cp = elacp_decode(desc, desc_len);
    if (!cp)
        return -1;

    if (elacp_get_type(cp) != ELACP_TYPE_USERINFO) {
        elacp_free(cp);
        vlogE("Carrier: Unkown userinfo type format.");
        return -1;
    }

    has_avatar = elacp_get_has_avatar(cp);

    name   = elacp_get_name(cp)   ? elacp_get_name(cp) : "";
    descr  = elacp_get_descr(cp)  ? elacp_get_descr(cp) : "";
    gender = elacp_get_gender(cp) ? elacp_get_gender(cp) : "";
    phone  = elacp_get_phone(cp)  ? elacp_get_phone(cp) : "";
    email  = elacp_get_email(cp)  ? elacp_get_email(cp) : "";
    region = elacp_get_region(cp) ? elacp_get_region(cp) : "";

    if (strcmp(info->name, name)) {
        strcpy(info->name, name);
        did_changed = true;
    }

    if (strcmp(info->description, descr)) {
        strcpy(info->description, descr);
        did_changed = true;
    }

    if (strcmp(info->gender, gender)) {
        strcpy(info->gender, gender);
        did_changed = true;
    }

    if (strcmp(info->phone, phone)) {
        strcpy(info->phone, phone);
        did_changed = true;
    }

    if (strcmp(info->email, email)) {
        strcpy(info->email, email);
        did_changed = true;
    }

    if (strcmp(info->region, region)) {
        strcpy(info->region, region);
        did_changed = true;
    }

    if (info->has_avatar != has_avatar) {
        info->has_avatar = has_avatar;
        did_changed = true;
    }

    elacp_free(cp);

    if (changed)
        *changed = did_changed;

    return 0;
}

static ElaPresenceStatus get_presence_status(int user_status)
{
    ElaPresenceStatus presence;

    if (user_status < ElaPresenceStatus_None)
        presence = ElaPresenceStatus_None;
    else if (user_status > ElaPresenceStatus_Busy)
        presence = ElaPresenceStatus_Busy;
    else
        presence = (ElaPresenceStatus)user_status;

    return presence;
}

static void get_self_info_cb(const uint8_t *address, const uint8_t *public_key,
                             int user_status,
                             const uint8_t *desc, size_t desc_len,
                             void *context)
{
    ElaCarrier *w = (ElaCarrier *)context;
    ElaUserInfo *ui = &w->me;
    size_t text_len;

    memcpy(w->address, address, DHT_ADDRESS_SIZE);
    memcpy(w->public_key, public_key, DHT_PUBLIC_KEY_SIZE);

    text_len = sizeof(w->base58_addr);
    base58_encode(address, DHT_ADDRESS_SIZE, w->base58_addr, &text_len);
    text_len = sizeof(ui->userid);
    base58_encode(public_key, DHT_PUBLIC_KEY_SIZE, ui->userid, &text_len);

    w->presence_status = get_presence_status(user_status);

    if (desc_len > 0)
        unpack_user_desc(desc, desc_len, ui, NULL);
    else
        fill_empty_user_desc(w);
}

static bool friends_iterate_cb(uint32_t friend_number,
                               const uint8_t *public_key,
                               int user_status,
                               const uint8_t *desc, size_t desc_len,
                               void *context)
{
    ElaCarrier *w = (ElaCarrier *)context;
    FriendInfo *fi;
    ElaUserInfo *ui;
    size_t _len = sizeof(ui->userid);
    int rc;

    assert(friend_number != UINT32_MAX);

    fi = (FriendInfo *)rc_zalloc(sizeof(FriendInfo), NULL);
    if (!fi)
        return false;

    ui = &fi->info.user_info;
    base58_encode(public_key, DHT_PUBLIC_KEY_SIZE, ui->userid, &_len);

    if (desc_len > 0) {
        rc = unpack_user_desc(desc, desc_len, ui, NULL);
        if (rc < 0) {
            deref(fi);
            return false;
        }
    }

    fi->info.status = ElaConnectionStatus_Disconnected;
    fi->info.presence = get_presence_status(user_status);
    fi->friend_number = friend_number;

    // Label will be synched later from data file.

    friends_put(w->friends, fi);

    deref(fi);

    return true;
}

static const char *data_filename = "eladata";

static int load_savedata(ElaCarrier *w)
{
    char *filename;
    FILE *fp;
    size_t datalen;
    uint8_t *data;
    int rc;
    int friend_sz;
    uint8_t *pos;
    int i;

    assert(w);
    assert(w->friends);

    rc = dht_get_self_info(&w->dht, get_self_info_cb, w);
    if (rc < 0)
        return rc;

    rc = dht_get_friends(&w->dht, friends_iterate_cb, w);
    if (rc < 0)
        return rc;

    filename = (char *)alloca(strlen(w->pref.data_location) + strlen(data_filename) + 4);
    sprintf(filename, "%s/%s", w->pref.data_location, data_filename);

    fp = fopen(filename, "r");
    // File 'eladata' was not created or lost. Along with later situation,
    // all labels of friends would be lost too, which should not be considered
    // as error when trying to load data.
    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_END);
    datalen = ftell(fp);
    rewind(fp);

    data = (uint8_t *)alloca(datalen);

    if (fread(data, sizeof(uint8_t), datalen, fp) != datalen) {
        vlogE("Carrier: Read save data file error.");
        //TODO: to set ela error number.
        //TOOD: should it be considered as error, or just truncate the file.
        fclose(fp);
        return -1;
    }

    fclose(fp);

    pos = data;
    if (memcmp(pos, w->public_key, DHT_PUBLIC_KEY_SIZE) != 0) {
        vlogE("Carrier: Deprecated carrier data.");
        return -1;
    }
    pos += DHT_PUBLIC_KEY_SIZE;

    friend_sz = *(uint32_t *)pos;
    pos += sizeof(uint32_t);

    for (i = 0; i < friend_sz; i++) {
        uint32_t friend_number;
        char *label;
        FriendInfo *fi;

        friend_number = *(uint32_t *)pos;
        pos += sizeof(uint32_t);
        label = (char *)pos;
        pos += strlen(label) + 1;

        fi = friends_get(w->friends, friend_number);
        if (fi) {
            strcpy(fi->info.label, label);
            deref(fi);
        }
    }

    return 0;
}

static int _mkdir(const char *path, mode_t mode)
{
    struct stat st;
    int rc = 0;

    if (stat(path, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            rc = -1;
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        rc = -1;
    }

    return rc;
}

static int mkdirs(const char *path, mode_t mode)
{
    int rc = 0;
    char *pp;
    char *sp;
    char copypath[PATH_MAX];

    strncpy(copypath, path, sizeof(copypath));
    copypath[sizeof(copypath) - 1] = 0;

    pp = copypath;
    while (rc == 0 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            /* Neither root nor double slash in path */
            *sp = '\0';
            rc = _mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }

    if (rc == 0)
        rc = _mkdir(path, mode);

    return rc;
}

static int store_savedata(ElaCarrier *w)
{
    HashtableIterator it;
    int count = 0;
    uint8_t *buf;
    size_t total_len = 0;
    uint8_t *pos;
    char *filename;
    FILE *fp;
    int rc;

    assert(w);
    assert(w->friends);

    total_len += DHT_ADDRESS_SIZE;
    total_len += sizeof(uint32_t);
    friends_iterate(w->friends, &it);
    while(friends_iterator_has_next(&it)) {
        FriendInfo *fi;

        if (friends_iterator_next(&it, &fi) == 1) {
            total_len += sizeof(uint32_t) + strlen(fi->info.label) + 1;
            count++;

            deref(fi);
        }
    }

    buf = (uint8_t *)calloc(1, total_len);
    if (!buf)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    pos = buf;
    memcpy(pos, w->public_key, DHT_PUBLIC_KEY_SIZE);
    pos += DHT_PUBLIC_KEY_SIZE;
    memcpy(pos, &count, sizeof(uint32_t));
    pos += sizeof(uint32_t);

    friends_iterate(w->friends, &it);
    while(friends_iterator_has_next(&it) && (pos - buf <= total_len)) {
        FriendInfo *fi;

        if (friends_iterator_next(&it, &fi) == 1) {
            memcpy(pos, &fi->friend_number, sizeof(uint32_t));
            pos += sizeof(uint32_t);
            memcpy(pos, fi->info.label, strlen(fi->info.label) + 1);
            pos += sizeof(fi->info.label) + 1;

            deref(fi);
        }
    }

    rc = mkdirs(w->pref.data_location, S_IRWXU);
    if (rc < 0) {
        free(buf);
        return ELA_GENERAL_ERROR(errno);
    }

    filename = (char *)alloca(strlen(w->pref.data_location) + strlen(data_filename) + 4);
    sprintf(filename, "%s/%s", w->pref.data_location, data_filename);

    fp = fopen(filename, "w");
    if (!fp) {
        free(buf);
        return ELA_GENERAL_ERROR(errno);
    }

    fwrite(buf, sizeof(uint8_t), pos - buf, fp);

    fclose(fp);
    free(buf);

    return 0;
}

static void ela_destroy(void *argv)
{
    ElaCarrier *w = (ElaCarrier *)argv;

    if (w->pref.data_location)
        free(w->pref.data_location);

    if (w->pref.bootstraps)
        free(w->pref.bootstraps);

    if (w->tcallbacks)
        deref(w->tcallbacks);

    if (w->thistory)
        deref(w->thistory);

    if (w->friends)
        deref(w->friends);

    dht_kill(&w->dht);
}

ElaCarrier *ela_new(const ElaOptions *opts,
                 ElaCallbacks *callbacks, void *context)
{
    ElaCarrier *w;
    int rc;
    int i;

    if (!opts) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    if (!opts->persistent_location || !*opts->persistent_location) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    w = (ElaCarrier *)rc_zalloc(sizeof(ElaCarrier), ela_destroy);
    if (!w) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    w->pref.udp_enabled = opts->udp_enabled;
    w->pref.data_location = strdup(opts->persistent_location);
    w->pref.bootstraps_size = opts->bootstraps_size;

    w->pref.bootstraps = (BootstrapNodeBuf *)calloc(1, sizeof(BootstrapNodeBuf)
                         * opts->bootstraps_size);
    if (!w->pref.bootstraps) {
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    for (i = 0; i < opts->bootstraps_size; i++) {
        BootstrapNode *b = &opts->bootstraps[i];
        BootstrapNodeBuf *bi = &w->pref.bootstraps[i];
        char *endptr = NULL;
        ssize_t len;

        if (b->ipv4 && strlen(b->ipv4) > MAX_IPV4_ADDRESS_LEN) {
            vlogE("Carrier: Bootstrap ipv4 address (%s) too long", b->ipv4);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        if (b->ipv6 && strlen(b->ipv6) > MAX_IPV6_ADDRESS_LEN) {
            vlogE("Carrier: Bootstrap ipv4 address (%s) too long", b->ipv6);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        if (!b->ipv4 && !b->ipv6) {
            vlogE("Carrier: Bootstrap ipv4 and ipv6 address both empty");
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        if (b->ipv4)
            strcpy(bi->ipv4, b->ipv4);
        if (b->ipv6)
            strcpy(bi->ipv6, b->ipv6);

        bi->port = (int)strtol(b->port, &endptr, 10);
        if (*endptr) {
            vlogE("Carrier: Invalid bootstrap port value (%s)", b->port);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        len = base58_decode(b->public_key, strlen(b->public_key), bi->public_key,
                            sizeof(bi->public_key));
        if (len != DHT_PUBLIC_KEY_SIZE) {
            vlogE("Carrier: Invalid bootstrap public key (%s)", b->public_key);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }
    }

    rc = dht_new(w->pref.data_location, w->pref.udp_enabled, &w->dht);
    if (rc < 0) {
        deref(w);
        ela_set_error(rc);
        return NULL;
    }

    w->friends = friends_create();
    if (!w->friends) {
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    w->tcallbacks = transacted_callbacks_create(32);
    if (!w->tcallbacks) {
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    w->thistory = transaction_history_create(32);
    if (!w->thistory) {
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    rc = load_savedata(w);
    if (rc < 0) {
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    rc = store_savedata(w);
    if (rc < 0) {
        deref(w);
        ela_set_error(rc);
        return NULL;
    }

    dht_store_savedata(&w->dht);

    srand((unsigned int)time(NULL));

    if (callbacks) {
        w->callbacks = *callbacks;
        w->context = context;
    }

    vlogI("Carrier: Carrier node created.");

    return w;
}

void ela_kill(ElaCarrier *w)
{
    if (!w) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return;
    }

    if (w->running) {
        w->quit = 1;

        if (!pthread_equal(pthread_self(), w->main_thread))
            while(!w->quit) usleep(5000);
    }

    dht_store_savedata(&w->dht);
    store_savedata(w);

    deref(w);

    vlogI("Carrier: Carrier node killed.");
}

static void notify_idle(ElaCarrier *w)
{
    if (w->callbacks.idle)
        w->callbacks.idle(w, w->context);
}

static void notify_friends(ElaCarrier *w)
{
    HashtableIterator it;

    friends_iterate(w->friends, &it);
    while(friends_iterator_has_next(&it)) {
        FriendInfo *fi;
        ElaFriendInfo _fi;

        if (friends_iterator_next(&it, &fi) == 1) {
            if (w->callbacks.friend_list) {
                memcpy(&_fi, &fi->info, sizeof(ElaFriendInfo));
                w->callbacks.friend_list(w, &_fi, w->context);
            }

            deref(fi);
        }
    }

    if (w->callbacks.friend_list)
        w->callbacks.friend_list(w, NULL, w->context);
}

static void notify_connection_cb(bool connected, void *context)
{
    ElaCarrier *w = (ElaCarrier *)context;

    if (!w->is_ready && connected) {
        w->is_ready = true;
        if (w->callbacks.ready)
            w->callbacks.ready(w, w->context);
    }

    w->connection_status = (connected ? ElaConnectionStatus_Connected :
                                        ElaConnectionStatus_Disconnected);

    if (w->callbacks.connection_status)
        w->callbacks.connection_status(w, w->connection_status, w->context);
}

static void notify_friend_info(ElaCarrier *w, uint32_t friend_number,
                               ElaFriendInfo *info)

{
    const char *userid;

    assert(w);
    assert(friend_number != UINT32_MAX);
    assert(info);

    userid = info->user_info.userid;

    if (friends_exist(w->friends, friend_number)) {
        if (w->callbacks.friend_info)
            w->callbacks.friend_info(w, userid, info, w->context);
    }
}

static
void notify_friend_description_cb(uint32_t friend_number, const uint8_t *desc,
                                  size_t length, void *context)
{
    ElaCarrier *w = (ElaCarrier *)context;
    FriendInfo *fi;
    ElaUserInfo *ui;
    bool changed;

    assert(friend_number != UINT32_MAX);
    assert(desc);

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number %lu, friend description message "
              "dropped.", friend_number);
        return;
    }

    if (length == 0) {
        vlogW("Carrier: Empty description message from friend "
              "number %lu, dropped.", friend_number);
        deref(fi);
        return;
    }

    ui = &fi->info.user_info;
    unpack_user_desc(desc, length, ui, &changed);

    if (changed) {
        ElaFriendInfo tmpfi;

        memcpy(&tmpfi, &fi->info, sizeof(tmpfi));
        notify_friend_info(w, friend_number, &tmpfi);
    }

    deref(fi);
}

static void notify_friend_connection(ElaCarrier *w, const char *friendid,
                                     ElaConnectionStatus status)
{
    assert(w);
    assert(friendid);

    if (w->callbacks.friend_connection)
        w->callbacks.friend_connection(w, friendid, status, w->context);
}

static
void notify_friend_connection_cb(uint32_t friend_number, bool connected,
                                 void *context)
{
    ElaCarrier *w = (ElaCarrier *)context;
    FriendInfo *fi;
    char tmpid[ELA_MAX_ID_LEN + 1];
    ElaConnectionStatus status;

    assert(friend_number != UINT32_MAX);

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number %lu, connection status message "
              "dropped (%s).", friend_number, connected ? "true":"false");
        return;
    }

    status = connected ? ElaConnectionStatus_Connected :
                         ElaConnectionStatus_Disconnected;

    if (status != fi->info.status) {
        fi->info.status = status;
        strcpy(tmpid, fi->info.user_info.userid);

        notify_friend_connection(w, tmpid, status);
    }

    deref(fi);
}

static void notify_friend_presence(ElaCarrier *w, const char *friendid,
                                   ElaPresenceStatus presence)
{
    assert(w);
    assert(friendid);

    if (w->callbacks.friend_presence)
        w->callbacks.friend_presence(w, friendid, presence, w->context);
}

static
void notify_friend_status_cb(uint32_t friend_number, int status,
                             void *context)
{
    ElaCarrier *w = (ElaCarrier *)context;
    FriendInfo *fi;
    char tmpid[ELA_MAX_ID_LEN + 1];

    assert(friend_number != UINT32_MAX);

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number (%lu), friend presence message "
              "dropped.", friend_number);
        return;
    }

    if (status < ElaPresenceStatus_None ||
        status > ElaPresenceStatus_Busy) {
        vlogE("Carrier: Invalid friend status %d received, dropped it.", status);
        return;
    }

    if (status != fi->info.presence) {
        fi->info.presence = status;
        strcpy(tmpid, fi->info.user_info.userid);

        notify_friend_presence(w, tmpid, status);
    }

    deref(fi);
}

static
void notify_friend_request_cb(const uint8_t *public_key, const uint8_t* gretting,
                              size_t length, void *context)
{
    ElaCarrier *w = (ElaCarrier *)context;
    uint32_t friend_number;
    ElaCP* cp;
    ElaUserInfo ui;
    size_t _len = sizeof(ui.userid);
    const char *name;
    const char *descr;
    const char *hello;
    int rc;

    assert(public_key);
    assert(gretting && length > 0);

    rc = dht_get_friend_number(&w->dht, public_key, &friend_number);
    if (rc == 0 && friend_number != UINT32_MAX) {
        vlogW("Carrier: friend already exist, dropped friend request.");
        return;
    }

    cp = elacp_decode(gretting, length);
    if (!cp) {
        vlogE("Carrier: Inavlid friend request, dropped this request.");
        return;
    }

    if (elacp_get_type(cp) != ELACP_TYPE_FRIEND_REQUEST) {
        vlogE("Carrier: Invalid friend request, dropped this request.");
        elacp_free(cp);
        return;
    }

    memset(&ui, 0, sizeof(ui));
    base58_encode(public_key, DHT_PUBLIC_KEY_SIZE, ui.userid, &_len);

    name  = elacp_get_name(cp)  ? elacp_get_name(cp)  : "";
    descr = elacp_get_descr(cp) ? elacp_get_descr(cp) : "";
    hello = elacp_get_hello(cp) ? elacp_get_hello(cp) : "";

    if (!*name)
        strcpy(ui.name, name);
    if (!*descr)
        strcpy(ui.description, descr);

    if (w->callbacks.friend_request)
        w->callbacks.friend_request(w, ui.userid, &ui, hello, w->context);

    elacp_free(cp);
}

static void notify_friend_added(ElaCarrier *w, ElaFriendInfo *fi)
{
    ElaFriendInfo tmpfi;

    assert(w);
    assert(fi);

    dht_store_savedata(&w->dht);
    store_savedata(w);

    memcpy(&tmpfi, fi, sizeof(tmpfi));

    if (w->callbacks.friend_added)
        w->callbacks.friend_added(w, &tmpfi, w->context);
}

static void notify_friend_removed(ElaCarrier *w, const char *friendid,
                                  ElaConnectionStatus status)
{
    assert(w);
    assert(friendid);

    dht_store_savedata(&w->dht);
    store_savedata(w);

    if (status == ElaConnectionStatus_Connected) {
        if (w->callbacks.friend_connection)
            w->callbacks.friend_connection(w, friendid,
                                ElaConnectionStatus_Disconnected, w->context);
    }

    if (w->callbacks.friend_removed)
        w->callbacks.friend_removed(w, friendid, w->context);
}

static
void handle_friend_message(ElaCarrier *w, uint32_t friend_number, ElaCP *cp)
{
    FriendInfo *fi;
    char friendid[ELA_MAX_ID_LEN + 1];
    const char *name;
    const char *msg;
    size_t len;

    assert(w);
    assert(friend_number != UINT32_MAX);
    assert(cp);
    assert(elacp_get_type(cp) == ELACP_TYPE_MESSAGE);

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number %lu, friend message dropped.",
              friend_number);
        return;
    }

    strcpy(friendid, fi->info.user_info.userid);
    deref(fi);

    name = elacp_get_extension(cp);
    msg  = elacp_get_raw_data(cp);
    len  = elacp_get_raw_data_length(cp);

    if (!name) {
        if (w->callbacks.friend_message)
            w->callbacks.friend_message(w, friendid, msg, len, w->context);
    }
}

static
void handle_invite_request(ElaCarrier *w, uint32_t friend_number, ElaCP *cp)
{
    FriendInfo *fi;
    char friendid[ELA_MAX_ID_LEN + 1];
    const char *name;
    const void *data;
    size_t len;
    int64_t tid;
    char from[ELA_MAX_ID_LEN + ELA_MAX_EXTENSION_NAME_LEN + 4];

    assert(w);
    assert(friend_number != UINT32_MAX);
    assert(cp);
    assert(elacp_get_type(cp) == ELACP_TYPE_INVITE_REQUEST);

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number %lu, invite request dropped.",
              friend_number);
        return;
    }

    strcpy(friendid, fi->info.user_info.userid);
    deref(fi);

    name = elacp_get_extension(cp);
    data = elacp_get_raw_data(cp);
    len  = elacp_get_raw_data_length(cp);
    tid  = elacp_get_tid(cp);

    strcpy(from, friendid);
    if (name) {
        strcat(from, ":");
        strcat(from, name);
    }
    tansaction_history_put_invite(w->thistory, from, tid);

    if (name) {
        if (strcmp(name, "session") == 0) {
            SessionExtension *ext = (SessionExtension *)w->session;
            if (ext && ext->friend_invite_cb)
                ext->friend_invite_cb(w, friendid, data, len, ext);
        }
    } else {
        if (w->callbacks.friend_invite)
            w->callbacks.friend_invite(w, friendid, data, len, w->context);
    }
}

static
void handle_invite_response(ElaCarrier *w, uint32_t friend_number, ElaCP *cp)
{
    FriendInfo *fi;
    char friendid[ELA_MAX_ID_LEN + 1];
    TransactedCallback *tcb;
    ElaFriendInviteResponseCallback *callback_func;
    void *callback_ctxt;
    int64_t tid;
    int status;
    const char *reason = NULL;
    const void *data = NULL;
    size_t data_len = 0;

    assert(w);
    assert(friend_number != UINT32_MAX);
    assert(cp);
    assert(elacp_get_type(cp) == ELACP_TYPE_INVITE_RESPONSE);

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number %lu, invite response dropped.",
              friend_number);
        return;
    }

    strcpy(friendid, fi->info.user_info.userid);
    deref(fi);

    tid = elacp_get_tid(cp);
    tcb = transacted_callbacks_get(w->tcallbacks, tid);
    if (!tcb) {
        vlogE("Carrier: No transaction to handle invite response.");
        return;
    }

    callback_func = (ElaFriendInviteResponseCallback *)tcb->callback_func;
    callback_ctxt = tcb->callback_context;
    assert(callback_func);

    deref(tcb);
    transacted_callbacks_remove(w->tcallbacks, tid);

    status = elacp_get_status(cp);
    if (status) {
        reason = elacp_get_reason(cp);
    } else {
        data = elacp_get_raw_data(cp);
        data_len = elacp_get_raw_data_length(cp);
    }

    callback_func(w, friendid, status, reason, data, data_len, callback_ctxt);
}

static
void notify_friend_message_cb(uint32_t friend_number, const uint8_t *message,
                              size_t length, void *context)
{
    ElaCarrier *w = (ElaCarrier *)context;
    ElaCP *cp;

    cp = elacp_decode(message, length);
    if (!cp) {
        vlogE("Carrier: Invalid DHT message, dropped.");
        return;
    }

    switch(elacp_get_type(cp)) {
    case ELACP_TYPE_MESSAGE:
        handle_friend_message(w, friend_number, cp);
        break;
    case ELACP_TYPE_INVITE_REQUEST:
        handle_invite_request(w, friend_number, cp);
        break;
    case ELACP_TYPE_INVITE_RESPONSE:
        handle_invite_response(w, friend_number, cp);
        break;
    default:
        vlogE("Carrier: Unknown DHT message, dropped.");
        break;
    }

    elacp_free(cp);
}

static void connect_to_bootstraps(ElaCarrier *w)
{
    int i;

    for (i = 0; i < w->pref.bootstraps_size; i++) {
        BootstrapNodeBuf *bi = &w->pref.bootstraps[i];
        char id[ELA_MAX_ID_LEN + 1] = {0};
        size_t id_len = sizeof(id);
        int rc;

        base58_encode(bi->public_key, DHT_PUBLIC_KEY_SIZE, id, &id_len);
        rc = dht_bootstrap(&w->dht, bi->ipv4, bi->ipv6, bi->port, bi->public_key);
        if (rc < 0) {
            vlogW("Carrier: Try to connect to bootstrap "
                  "[ipv4:%s, ipv6:%s, port:%d, public_key:%s] error.",
                  *bi->ipv4 ? bi->ipv4 : "N/A", *bi->ipv6 ? bi->ipv6 : "N/A",
                  bi->port, id);
        } else {
            vlogT("Carrier: Try to connect to bootstrap "
                  "[ipv4:%s, ipv6:%s, port:%d, public_key:%s] succeess.",
                  *bi->ipv4 ? bi->ipv4 : "N/A", *bi->ipv6 ? bi->ipv6 : "N/A",
                  bi->port, id);
        }
    }
}

int ela_run(ElaCarrier *w, int interval)
{
    if (!w || interval < 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (interval == 0)
        interval = 1000; // in milliseconds.

    ref(w);

    w->dht_callbacks.notify_connection = notify_connection_cb;
    w->dht_callbacks.notify_friend_desc = notify_friend_description_cb;
    w->dht_callbacks.notify_friend_connection = notify_friend_connection_cb;
    w->dht_callbacks.notify_friend_status = notify_friend_status_cb;
    w->dht_callbacks.notify_friend_request = notify_friend_request_cb;
    w->dht_callbacks.notify_friend_message = notify_friend_message_cb;
    w->dht_callbacks.context = w;

    notify_friends(w);

    w->running = 1;

    connect_to_bootstraps(w);

    while(!w->quit) {
        int idle_interval;
        struct timeval expire;
        struct timeval check;
        struct timeval tmp;

        gettimeofday(&expire, NULL);

        idle_interval = dht_iteration_idle(&w->dht);
        if (idle_interval > interval)
            idle_interval = interval;

        tmp.tv_sec = 0;
        tmp.tv_usec = idle_interval * 1000;

        timeradd(&expire, &tmp, &expire);

        if (idle_interval > 0)
            notify_idle(w);

        // TODO: Check connection:.

        gettimeofday(&check, NULL);

        if (timercmp(&expire, &check, >)) {
            timersub(&expire, &check, &tmp);
            usleep(tmp.tv_usec);
        }

        dht_iterate(&w->dht, &w->dht_callbacks);
    }

    w->running = 0;

    deref(w);

    return 0;
}

char *ela_get_address(ElaCarrier *w, char *address, size_t length)
{
    if (!w || !address || !length) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    if (strlen(w->base58_addr) >= length) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_BUFFER_TOO_SMALL));
        return NULL;
    }

    strcpy(address, w->base58_addr);
    return address;
}

char *ela_get_nodeid(ElaCarrier *w, char *nodeid, size_t len)
{
    if (!w || !nodeid || !len) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    if (strlen(w->me.userid) >= len) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_BUFFER_TOO_SMALL));
        return NULL;
    }

    strcpy(nodeid, w->me.userid);
    return nodeid;
}

char *ela_get_userid(ElaCarrier *w, char *userid, size_t len)
{
    return ela_get_nodeid(w, userid, len);
}

int ela_set_self_nospam(ElaCarrier *w, uint32_t nospam)
{
    int rc;

    if (!w) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    dht_self_set_nospam(&w->dht, nospam);

    rc = dht_get_self_info(&w->dht, get_self_info_cb, w);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    dht_store_savedata(&w->dht);
    
    return 0;
}

int ela_get_self_nospam(ElaCarrier *w, uint32_t *nospam)
{
    if (!w || !nospam) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    *nospam = dht_self_get_nospam(&w->dht);

    return 0;
}

int ela_set_self_info(ElaCarrier *w, const ElaUserInfo *info)
{
    ElaCP *cp;
    uint8_t *data;
    size_t data_len;
    bool did_changed = false;

    if (!w || !info) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (strcmp(info->userid, w->me.userid) != 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    cp = elacp_create(ELACP_TYPE_USERINFO, NULL);
    if (!cp) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    if (info->has_avatar != w->me.has_avatar ||
        strcmp(info->name, w->me.name) ||
        strcmp(info->description, w->me.description) ||
        strcmp(info->gender, w->me.gender) ||
        strcmp(info->phone, w->me.phone) ||
        strcmp(info->email, w->me.email) ||
        strcmp(info->region, w->me.region)) {
        did_changed = true;
    } else {
        elacp_free(cp);
    }

    if (did_changed) {
        elacp_set_has_avatar(cp, !!info->has_avatar);
        elacp_set_name(cp, info->name);
        elacp_set_descr(cp, info->description);
        elacp_set_gender(cp, info->gender);
        elacp_set_phone(cp, info->phone);
        elacp_set_email(cp, info->email);
        elacp_set_region(cp, info->region);

        data = elacp_encode(cp, &data_len);
        elacp_free(cp);

        if (!data) {
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
            return -1;
        }

        /* Use tox status message as user information. The total length of
           user information is about 700, far less than the max length
           value of status message (1007).
         */
        w->me.has_avatar = info->has_avatar;
        strcpy(w->me.name, info->name);
        strcpy(w->me.description, info->description);
        strcpy(w->me.gender, info->gender);
        strcpy(w->me.phone, info->phone);
        strcpy(w->me.email, info->email);
        strcpy(w->me.region, info->region);
        dht_self_set_desc(&w->dht, data, data_len);
        dht_store_savedata(&w->dht);

        free(data);
    }

    return 0;
}

int ela_get_self_info(ElaCarrier *w, ElaUserInfo *info)
{
    if (!w || !info) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    memcpy(info, &w->me, sizeof(ElaUserInfo));

    return 0;
}

int ela_set_self_presence(ElaCarrier *w, ElaPresenceStatus status)
{
    if (!w) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (status < ElaPresenceStatus_None ||
        status > ElaPresenceStatus_Busy) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    dht_self_set_status(&w->dht, (int)status);

    return 0;
}

int ela_get_self_presence(ElaCarrier *w, ElaPresenceStatus *status)
{
    int presence_status;

    if (!w || !status) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    presence_status = dht_self_get_status(&w->dht);

    if (presence_status < ElaPresenceStatus_None)
        *status = ElaPresenceStatus_None;
    else if (presence_status > ElaPresenceStatus_Busy)
        *status = ElaPresenceStatus_None;
    else
        *status = (ElaPresenceStatus)presence_status;

    return 0;
}

bool ela_is_ready(ElaCarrier *w)
{
    if (!w) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return false;
    }

    return w->is_ready;
}

int ela_get_friends(ElaCarrier *w,
                    ElaFriendsIterateCallback *callback, void *context)
{
    HashtableIterator it;

    if (!w || !callback) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    friends_iterate(w->friends, &it);
    while(friends_iterator_has_next(&it)) {
        FriendInfo *fi;

        if (friends_iterator_next(&it, &fi) == 1) {
            ElaFriendInfo wfi;

            memcpy(&wfi, &fi->info, sizeof(ElaFriendInfo));
            deref(fi);

            if (!callback(&wfi, context))
                return 0;
        }
    }

    /* Friend list is end */
    callback(NULL, context);

    return 0;
}

int ela_get_friend_info(ElaCarrier *w, const char *friendid,
                        ElaFriendInfo *info)
{
    uint32_t friend_number;
    FriendInfo *fi;
    int rc;

    if (!w || !friendid || !info) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    rc = get_friend_number(w, friendid, &friend_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }
    assert(!strcmp(friendid, fi->info.user_info.userid));

    memcpy(info, &fi->info, sizeof(ElaFriendInfo));

    deref(fi);

    return 0;
}

int ela_set_friend_label(ElaCarrier *w,
                             const char *friendid, const char *label)
{
    uint32_t friend_number;
    FriendInfo *fi;
    int rc;

    if (!w || !friendid) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (label && strlen(label) > ELA_MAX_USER_NAME_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    rc = get_friend_number(w, friendid, &friend_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }
    assert(!strcmp(friendid, fi->info.user_info.userid));

    strcpy(fi->info.label, label ? label : "");

    deref(fi);

    store_savedata(w); // saved label just updated.

    return 0;
}

bool ela_is_friend(ElaCarrier *w, const char *userid)
{
    uint32_t friend_number;
    int rc;

    if (!w || !userid) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return false;
    }

    rc = get_friend_number(w, userid, &friend_number);
    if (rc < 0 || friend_number == UINT32_MAX) {
        ela_set_error(rc);
        return false;
    }

    return !!friends_exist(w->friends, friend_number);
}

int ela_add_friend(ElaCarrier *w, const char *address, const char *hello)
{
    uint32_t friend_number;
    FriendInfo *fi;
    uint8_t addr[DHT_ADDRESS_SIZE];
    ElaCP *cp;
    uint8_t *data;
    size_t data_len;
    size_t _len;
    int rc;

    if (!w || !hello || !address) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!is_valid_address(address)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!strcmp(address, w->base58_addr)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    base58_decode(address, strlen(address), addr, sizeof(addr));

    rc = dht_get_friend_number(&w->dht, addr, &friend_number);
    if (rc == 0 && friend_number != UINT32_MAX) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST));
        return -1;
    }

    fi = (FriendInfo *)rc_zalloc(sizeof(FriendInfo), NULL);
    if (!fi) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    cp = elacp_create(ELACP_TYPE_FRIEND_REQUEST, NULL);
    if (!cp) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        deref(fi);
        return -1;
    }

    elacp_set_name(cp, w->me.name);
    elacp_set_descr(cp, w->me.description);
    elacp_set_hello(cp, hello);

    data = elacp_encode(cp, &data_len);
    elacp_free(cp);

    if (!data) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        deref(fi);
        return -1;
    }

    rc = dht_friend_add(&w->dht, addr, data, data_len, &friend_number);
    free(data);

    if (rc < 0) {
        ela_set_error(rc);
        deref(fi);
        return -1;
    }

    _len = sizeof(fi->info.user_info.userid);
    base58_encode(addr, DHT_PUBLIC_KEY_SIZE, fi->info.user_info.userid, &_len);

    fi->friend_number = friend_number;
    fi->info.presence = ElaPresenceStatus_None;
    fi->info.status   = ElaConnectionStatus_Disconnected;
    friends_put(w->friends, fi);

    notify_friend_added(w, &fi->info);

    deref(fi);

    return 0;
}

int ela_accept_friend(ElaCarrier *w, const char *userid)
{
    uint32_t friend_number = UINT32_MAX;
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
    FriendInfo *fi;
    int rc;

    if (!w || !userid) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!is_valid_key(userid)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (strcmp(userid, w->me.userid) == 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_friend_number(w, userid, &friend_number);
    if (rc == 0 && friend_number != UINT32_MAX) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST));
        return -1;
    }

    fi = (FriendInfo *)rc_zalloc(sizeof(FriendInfo), NULL);
    if (!fi) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    base58_decode(userid, strlen(userid), public_key, sizeof(public_key));
    rc = dht_friend_add_norequest(&w->dht, public_key, &friend_number);
    if (rc < 0) {
        deref(fi);
        ela_set_error(rc);
        return -1;
    }

    strcpy(fi->info.user_info.userid, userid);

    fi->friend_number = friend_number;
    fi->info.presence = ElaPresenceStatus_None;
    fi->info.status   = ElaConnectionStatus_Disconnected;

    friends_put(w->friends, fi);

    notify_friend_added(w, &fi->info);

    deref(fi);

    return 0;
}

int ela_remove_friend(ElaCarrier *w, const char *friendid)
{
    uint32_t friend_number;
    FriendInfo *fi;
    ElaConnectionStatus status;
    int rc;

    if (!w || !friendid) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!is_valid_key(friendid)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_friend_number(w, friendid, &friend_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    if (!friends_exist(w->friends, friend_number)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    dht_friend_delete(&w->dht, friend_number);

    fi = friends_remove(w->friends, friend_number);
    assert(fi);

    status = fi->info.status;

    deref(fi);

    notify_friend_removed(w, friendid, status);

    return 0;
}

static void parse_address(const char *addr, char **uid, char **ext)
{
    char *colon_pos = NULL;

    assert(addr);
    assert(uid && ext);

    /* address format: userid:extenison */
    if (uid)
        *uid = (char *)addr;

    colon_pos = strchr(addr, ':');
    if (colon_pos) {
        if (ext)
            *ext = colon_pos+1;
        *colon_pos = 0;
    } else {
        if (ext)
            *ext = NULL;
    }
}

int ela_send_friend_message(ElaCarrier *w, const char *to, const char *msg,
                            size_t len)
{
    char *addr, *userid, *ext_name;
    uint32_t friend_number;
    int rc;
    ElaCP *cp;
    uint8_t *data;
    size_t data_len;

    if (!w || !to || !msg || !len || len > ELA_MAX_APP_MESSAGE_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    addr = alloca(strlen(to) + 1);
    strcpy(addr, to);
    parse_address(addr, &userid, &ext_name);

    if (!is_valid_key(userid)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (ext_name && strlen(ext_name) > ELA_MAX_USER_NAME_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (strcmp(userid, w->me.userid) == 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        vlogE("Carrier: Send message to myself not allowed.");
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_friend_number(w, userid, &friend_number);
    if (rc < 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (!friends_exist(w->friends, friend_number)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    cp = elacp_create(ELACP_TYPE_MESSAGE, ext_name);
    if (!cp) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    elacp_set_raw_data(cp, msg, len);

    data = elacp_encode(cp, &data_len);
    elacp_free(cp);

    if (!data) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    rc = dht_friend_message(&w->dht, friend_number, data, data_len);
    free(data);

    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    return 0;
}

static int64_t generate_tid(void)
{
    int64_t tid;

    do {
        tid = time(NULL);
        tid += rand();
    } while (tid == 0);

    return tid;
}

int ela_invite_friend(ElaCarrier *w, const char *to,
                      const char *data, size_t len,
                      ElaFriendInviteResponseCallback *callback,
                      void *context)
{
    char *addr, *userid, *ext_name;
    uint32_t friend_number;
    ElaCP *cp;
    int rc;
    TransactedCallback *tcb;
    int64_t tid;
    uint8_t *_data;
    size_t _data_len;

    if (!w || !to || !data || !len || !callback) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    addr = alloca(strlen(to) + 1);
    strcpy(addr, to);
    parse_address(addr, &userid, &ext_name);

    if (!is_valid_key(userid)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (ext_name && strlen(ext_name) > ELA_MAX_USER_NAME_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_friend_number(w, userid, &friend_number);
    if (rc < 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (!friends_exist(w->friends, friend_number)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    cp = elacp_create(ELACP_TYPE_INVITE_REQUEST, ext_name);
    if (!cp) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    tid = generate_tid();

    elacp_set_tid(cp, &tid);
    elacp_set_raw_data(cp, data, len);

    _data = elacp_encode(cp, &_data_len);
    elacp_free(cp);

    if (!_data) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    tcb = (TransactedCallback*)rc_alloc(sizeof(TransactedCallback), NULL);
    if (!tcb) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        free(_data);
        return -1;
    }

    tcb->tid = tid;
    tcb->callback_func = callback;
    tcb->callback_context = context;

    transacted_callbacks_put(w->tcallbacks, tcb);
    deref(tcb);

    rc = dht_friend_message(&w->dht, friend_number, _data, _data_len);
    free(_data);

    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    return 0;
}

int ela_reply_friend_invite(ElaCarrier *w, const char *to,
                                int status, const char *reason,
                                const char *data, size_t len)
{
    char *addr, *userid, *ext_name;
    uint32_t friend_number;
    int64_t tid;
    ElaCP *cp;
    int rc;
    uint8_t *_data;
    size_t _data_len;

    if (!w || !to || !*to || (status != 0 && !reason)
            || (status == 0 && (!data || !len))) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    addr = alloca(strlen(to) + 1);
    strcpy(addr, to);
    parse_address(addr, &userid, &ext_name);

    if (!is_valid_key(userid)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (ext_name && strlen(ext_name) > ELA_MAX_USER_NAME_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_friend_number(w, userid, &friend_number);
    if (rc < 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!friends_exist(w->friends, friend_number)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    tid = transaction_history_get_invite(w->thistory, to);
    if (tid == 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NO_MATCHED_REQUEST));
        return -1;
    }

    cp = elacp_create(ELACP_TYPE_INVITE_RESPONSE, ext_name);
    if (!cp) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    elacp_set_tid(cp, &tid);
    elacp_set_status(cp, status);
    if (status)
        elacp_set_reason(cp, reason);
    else
        elacp_set_raw_data(cp, data, len);

    _data = elacp_encode(cp, &_data_len);
    elacp_free(cp);

    if (!_data) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    rc = dht_friend_message(&w->dht, friend_number, _data, _data_len);
    free(_data);

    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    transaction_history_remove_invite(w->thistory, to);

    return 0;
}

int ela_get_turn_server(ElaCarrier *w, ElaTurnServer *turn_server)
{
    uint8_t secret_key[PUBLIC_KEY_BYTES];
    uint8_t public_key[PUBLIC_KEY_BYTES];
    uint8_t shared_key[SYMMETRIC_KEY_BYTES];
    uint8_t nonce[NONCE_BYTES];
    uint8_t digest[SHA256_BYTES];
    char nonce_str[64];
    size_t text_len;
    int rc;
    int times = 0;

    if (!w || !turn_server) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

redo_get_tcp_relay:
    rc = dht_get_random_tcp_relay(&w->dht, turn_server->server,
                                  sizeof(turn_server->server), public_key);
    if (rc < 0) {
        if (++times < 5) {
            usleep(1000);
            //BUGBUG: Try to get tcp relay again and again.
            goto redo_get_tcp_relay;
        } else {
            vlogE("Carrier: Get turn server address and public key error (%d)", rc);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
            return -1;
        }
    }

    {
        char bootstrap_pk[ELA_MAX_ID_LEN + 1];
        size_t pk_len = sizeof(bootstrap_pk);

        base58_encode(public_key, sizeof(public_key), bootstrap_pk, &pk_len);
        vlogD("Carrier: Acquired a random tcp relay (ip: %s, pk: %s)",
              turn_server->server, bootstrap_pk);
    }

    turn_server->port = TURN_SERVER_PORT;

    dht_self_get_secret_key(&w->dht, secret_key);
    crypto_compute_symmetric_key(public_key, secret_key, shared_key);

    crypto_random_nonce(nonce);
    rc = (int)hmac_sha256(shared_key, sizeof(shared_key),
                          nonce, sizeof(nonce), digest, sizeof(digest));

    memset(secret_key, 0, sizeof(secret_key));
    memset(shared_key, 0, sizeof(shared_key));

    if (rc != sizeof(digest)) {
        vlogE("Carrier: Hmac sha256 to nonce error");
        return -1;
    }

    text_len = sizeof(turn_server->password);
    base58_encode(digest, sizeof(digest), turn_server->password, &text_len);

    text_len = sizeof(nonce_str);
    base58_encode(nonce, sizeof(nonce), nonce_str, &text_len);

    sprintf(turn_server->username, "%s@%s.%s", w->me.userid, nonce_str, TURN_SERVER_USER_SUFFIX);

    strcpy(turn_server->realm, TURN_REALM);

    vlogD("Carrier: Valid turn server information: >>>>");
    vlogD("    host: %s", turn_server->server);
    vlogD("    port: %hu", turn_server->port);
    vlogD("   realm: %s", turn_server->realm);
    vlogD("username: %s", turn_server->username);
    vlogD("password: %s", turn_server->password);
    vlogD("<<<<");

    return 0;
}

#if defined(_WIN32) || defined(_WIN64)
#define __thread        __declspec(thread)
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
static __thread int ela_error;
#elif defined(__APPLE__)
#include <pthread.h>
static pthread_once_t ela_key_once = PTHREAD_ONCE_INIT;
static pthread_key_t ela_error;
static void ela_setup_error(void)
{
    (void)pthread_key_create(&ela_error, NULL);
}
#else
#error "Unsupported OS yet"
#endif

int ela_get_error(void)
{
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
    return ela_error;
#elif defined(__APPLE__)
    return (int)pthread_getspecific(ela_error);
#else
#error "Unsupported OS yet"
#endif
}

void ela_clear_error(void)
{
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
    ela_error = ELASUCCESS;
#elif defined(__APPLE__)
    (void)pthread_setspecific(ela_error, 0);
#else
#error "Unsupported OS yet"
#endif
}

void ela_set_error(int err)
{
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
    ela_error = err;
#elif defined(__APPLE__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    (void)pthread_once(&ela_key_once, ela_setup_error);
    (void)pthread_setspecific(ela_error, (void*)err);
#pragma GCC diagnostic pop
#else
#error "Unsupported OS yet"
#endif
}


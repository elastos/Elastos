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
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <sys/syslimits.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <crystal.h>

#if !(defined(_WIN32) || defined(_WIN64))
#define O_BINARY 0
#endif

#include "version.h"
#include "ela_carrier.h"
#include "ela_carrier_impl.h"
#include "ela_turnserver.h"
#include "friends.h"
#include "tcallbacks.h"
#include "thistory.h"
#include "elacp.h"
#include "dht.h"
#include "tassemblies.h"
#include "dstore_wrapper.h"

#define TURN_SERVER_PORT                ((uint16_t)3478)
#define TURN_SERVER_USER_SUFFIX         "auth.tox"
#define TURN_REALM                      "elastos.org"

#define TASSEMBLY_TIMEOUT               (60) //60s.

// Carrier invite request/response data transmission unit length.
#define INVITE_DATA_UNIT                (1280)

const char* ela_get_version(void)
{
    return carrier_version;
}

static bool is_valid_key(const char *key)
{
    char result[DHT_PUBLIC_KEY_SIZE];
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
    char dht_name[ELA_MAX_USER_NAME_LEN + 1];
    int name_len;

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

    name_len = dht_self_get_name(&w->dht, (uint8_t *)dht_name,
                                 sizeof(dht_name));
    if (name_len < 0)
        return;

    if ((name_len <= 1 && !*w->me.name) ||
        (name_len > 1 && !strcmp(dht_name, w->me.name)))
        return;

    dht_self_set_name(&w->dht, (uint8_t *)w->me.name, strlen(w->me.name) + 1);
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

static const uint32_t PERSISTENCE_MAGIC = 0x0E0C0D0A;
static const uint32_t PERSISTENCE_REVISION = 2;

static const char *data_filename = "carrier.data";
static const char *old_dhtdata_filename = "dhtdata";
static const char *old_eladata_filename = "eladata";

#define MAX_PERSISTENCE_SECTION_SIZE        (16 * 1024 *1024)

#define ROUND256(s)     (((((s) + 64) >> 8) + 1) << 8)

typedef struct persistence_data {
    size_t dht_savedata_len;
    const uint8_t *dht_savedata;
    size_t extra_savedata_len;
    const uint8_t *extra_savedata;
} persistence_data;

static int convert_old_dhtdata(const char *data_location)
{
    uint8_t *buf;
    uint8_t *pos;
    char *dhtdata_filename;
    char *eladata_filename;
    char *journal_filename;
    char *filename;
    struct stat st;
    uint32_t val;
    int fd;

    size_t dht_data_len;
    size_t extra_data_len;
    size_t total_len;

    assert(data_location);

    dhtdata_filename = (char *)alloca(strlen(data_location) + strlen(old_dhtdata_filename) + 4);
    sprintf(dhtdata_filename, "%s/%s", data_location, old_dhtdata_filename);
    eladata_filename = (char *)alloca(strlen(data_location) + strlen(old_eladata_filename) + 4);
    sprintf(eladata_filename, "%s/%s", data_location, old_eladata_filename);

    if (stat(dhtdata_filename, &st) < 0)
        return ELA_SYS_ERROR(errno);

    dht_data_len = st.st_size;

    if (stat(eladata_filename, &st) < 0 ||
            st.st_size < (PUBLIC_KEY_BYTES + sizeof(uint32_t)))
        extra_data_len = 0;
    else
        extra_data_len = (st.st_size - PUBLIC_KEY_BYTES - sizeof(uint32_t));

    total_len = 256 + ROUND256(dht_data_len) + ROUND256(extra_data_len);
    buf = (uint8_t *)calloc(total_len, 1);
    if (!buf)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    pos = buf + 256;

    fd = open(dhtdata_filename, O_RDONLY | O_BINARY);
    if (fd < 0) {
        free(buf);
        return ELA_SYS_ERROR(errno);
    }

    if (read(fd, pos, dht_data_len) != dht_data_len) {
        free(buf);
        close(fd);
        return ELA_SYS_ERROR(errno);
    }

    close(fd);

    if (extra_data_len) {
        pos += ROUND256(dht_data_len);

        fd = open(eladata_filename, O_RDONLY | O_BINARY);
        if (fd < 0) {
            extra_data_len = 0;
            goto write_data;
        }

        // Skip public key
        lseek(fd, PUBLIC_KEY_BYTES, SEEK_SET);
        // read friends count
        if (read(fd, &val, sizeof(val)) != sizeof(val)) {
            close(fd);
            extra_data_len = 0;
            goto write_data;
        }

        if (val > 0) {
            if (read(fd, pos, extra_data_len) != extra_data_len) {
                close(fd);
                memset(pos, 0, extra_data_len);
                extra_data_len = 0;
                goto write_data;
            }

            uint8_t *rptr = pos;
            uint8_t *wptr = pos;
            uint32_t i;

            for (i = 0; i < val; i++) {
                uint32_t id = *(uint32_t *)rptr;
                rptr += sizeof(uint32_t);
                size_t label_len = strlen((const char *)rptr);
                if (label_len == 0) {
                    rptr++;
                    continue;
                }

                id = htonl(id);
                memcpy(wptr, &id, sizeof(id));
                wptr += sizeof(uint32_t);
                memmove(wptr, rptr, label_len + 1);
                wptr += (label_len + 1);
                rptr += (label_len + 1);
            }

            extra_data_len = wptr - pos;
            memset(wptr, 0, rptr - wptr);
        }

        close(fd);
    }

write_data:

    total_len = 256 + ROUND256(dht_data_len) + ROUND256(extra_data_len);

    pos = buf;
    val = htonl(PERSISTENCE_MAGIC);
    memcpy(pos, &val, sizeof(uint32_t));

    pos += sizeof(uint32_t);
    val = htonl(PERSISTENCE_REVISION);
    memcpy(pos, &val, sizeof(uint32_t));

    pos += sizeof(uint32_t);
    val = htonl((uint32_t)dht_data_len);
    memcpy(pos, &val, sizeof(uint32_t));

    pos += sizeof(uint32_t);
    val = htonl((uint32_t)extra_data_len);
    memcpy(pos, &val, sizeof(uint32_t));

    pos = buf + 256;
    sha256(pos, total_len - 256, buf + (sizeof(uint32_t) * 4), SHA256_BYTES);

    filename = (char *)alloca(strlen(data_location) + strlen(data_filename) + 4);
    sprintf(filename, "%s/%s", data_location, data_filename);
    journal_filename = (char *)alloca(strlen(data_location) + strlen(data_filename) + 16);
    sprintf(journal_filename, "%s/%s.journal", data_location, data_filename);

    fd = open(journal_filename, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        free(buf);
        return ELA_SYS_ERROR(errno);
    }

    if (write(fd, buf, total_len) != total_len) {
        close(fd);
        remove(journal_filename);
        return ELA_SYS_ERROR(errno);
    }

    if (fsync(fd) < 0) {
        close(fd);
        remove(journal_filename);
        return ELA_SYS_ERROR(errno);
    }

    close(fd);
    free(buf);

    remove(dhtdata_filename);
    remove(eladata_filename);
    remove(filename);
    rename(journal_filename, filename);

    return 0;
}

static int _load_persistence_data_i(int fd, persistence_data *data)
{
    struct stat st;
    uint32_t val;
    size_t dht_data_len;
    size_t extra_data_len;
    unsigned char p_sum[SHA256_BYTES];
    unsigned char c_sum[SHA256_BYTES];

    uint8_t *buf;

    if (fstat(fd, &st) < 0) {
        vlogW("Load persistence data failed, stat files error(%d).", errno);
        return -1;
    }

    if (st.st_size < 256) {
        vlogW("Load persistence data failed, corrupt file.");
        return -1;
    }

    if (read(fd, (void *)&val, sizeof(val)) != sizeof(val)) {
        vlogW("Load persistence data failed, read error(%d).", errno);
        return -1;
    }
    val = ntohl(val);
    if (val != PERSISTENCE_MAGIC) {
        vlogW("Load persistence data failed, corrupt file.");
        return -1;
    }

    if (read(fd, (void *)&val, sizeof(val)) != sizeof(val)) {
        vlogW("Load persistence data failed, read error(%d).", errno);
        return -1;
    }
    val = ntohl(val);
    if (val != PERSISTENCE_REVISION) {
        vlogW("Load persistence data failed, unsupported date file version.");
        return -1;
    }

    if (read(fd, (void *)&val, sizeof(val)) != sizeof(val)) {
        vlogW("Load persistence data failed, read error(%d).", errno);
        return -1;
    }
    dht_data_len = ntohl(val);
    if (dht_data_len > MAX_PERSISTENCE_SECTION_SIZE) {
        vlogW("Load persistence data failed, corrupt file.");
        return -1;
    }

    if (read(fd, (void *)&val, sizeof(val)) != sizeof(val)) {
        vlogW("Load persistence data failed, read error(%d).", errno);
        return -1;
    }
    extra_data_len = ntohl(val);
    if (extra_data_len > MAX_PERSISTENCE_SECTION_SIZE) {
        vlogW("Load persistence data failed, corrupt file.");
        return -1;
    }

    if (st.st_size != 256 + ROUND256(dht_data_len) + ROUND256(extra_data_len)) {
        vlogW("Load persistence data failed, corrupt file.");
        return -1;
    }

    if (read(fd, p_sum, sizeof(p_sum)) != sizeof(p_sum)) {
        vlogW("Load persistence data failed, read error(%d).", errno);
        return -1;
    }

    buf = (uint8_t *)malloc(st.st_size - 256);
    if (!buf) {
        vlogW("Load persistence data failed, out of memory.");
        return -1;
    }

    lseek(fd, 256, SEEK_SET);
    if (read(fd, buf, st.st_size - 256) != (st.st_size - 256)) {
        vlogW("Load persistence data failed, read error(%d).", errno);
        free(buf);
        return -1;
    }

    sha256(buf, st.st_size - 256, c_sum, sizeof(c_sum));
    if (memcmp(p_sum, c_sum, SHA256_BYTES) != 0) {
        vlogW("Load persistence data failed, corrupt file.");
        free(buf);
        return -1;
    }

    data->dht_savedata_len = dht_data_len;
    data->dht_savedata = (const uint8_t *)buf;
    data->extra_savedata_len = extra_data_len;
    data->extra_savedata = (const uint8_t *)buf + ROUND256(dht_data_len);

    return 0;
}

#define FAILBACK_OR_RETURN(rc)          \
    if (journal) {                      \
        journal = 0;                    \
        goto failback;                  \
    } else {                            \
        return rc;                      \
    }

static int load_persistence_data(const char *data_location, persistence_data *data)
{
    char *filename;
    int journal = 1;
    int fd;
    int rc;

    assert(data_location);
    assert(data);

    filename = (char *)alloca(strlen(data_location) + strlen(data_filename) + 16);

failback:
    // Load from journal file first.
    if (journal)
        sprintf(filename, "%s/%s.journal", data_location, data_filename);
    else
        sprintf(filename, "%s/%s", data_location, data_filename);

    if (access(filename, F_OK | R_OK) < 0) {
        if (journal) {
            journal = 0;
            goto failback;
        } else {
            sprintf(filename, "%s/%s", data_location, old_dhtdata_filename);

            if (access(filename, F_OK | R_OK) < 0)
                return -1;

            vlogT("Try convert old persistence data...");
            if (convert_old_dhtdata(data_location) < 0) {
                vlogE("Convert old persistence data failed.");
                return -1;
            }

            vlogT("Convert old persistence data to current version.");
            goto failback;
        }
    }

    vlogD("Try to loading persistence data from: %s.", filename);

    fd = open(filename, O_RDONLY | O_BINARY);
    if (fd < 0) {
        vlogD("Loading persistence data failed, cannot open file.");
        FAILBACK_OR_RETURN(-1);
    }

    rc = _load_persistence_data_i(fd, data);
    close(fd);

    if (rc < 0) {
        FAILBACK_OR_RETURN(rc);
    }

    if (rc == 0 && journal) {
        char *journal_filename = filename;
        char *filename = (char *)alloca(strlen(data_location) + strlen(data_filename) + 16);
        sprintf(filename, "%s/%s", data_location, data_filename);

        remove(filename);
        rename(journal_filename, filename);
    }

    return rc;
}

static void apply_extra_data(ElaCarrier *w, const uint8_t *extra_savedata, size_t extra_savedata_len)
{
    const uint8_t *pos = extra_savedata;

    while (extra_savedata_len > 0) {
        uint32_t friend_number;
        char *label;
        size_t label_len;
        FriendInfo *fi;

        friend_number = ntohl(*(uint32_t *)pos);
        pos += sizeof(uint32_t);
        label = (char *)pos;
        label_len = strlen(label);
        pos += label_len + 1;

        if (label_len == 0)
            break;

        fi = friends_get(w->friends, friend_number);
        if (fi) {
            strcpy(fi->info.label, label);
            deref(fi);
        }

        extra_savedata_len -= (sizeof(uint32_t) + label_len + 1);
    }
}

static void free_persistence_data(persistence_data *data)
{
    if (data && data->dht_savedata)
        free((void *)data->dht_savedata);
}

static int mkdir_internal(const char *path, mode_t mode)
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
            rc = mkdir_internal(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }

    if (rc == 0)
        rc = mkdir_internal(path, mode);

    return rc;
}

static size_t get_extra_savedata_size(ElaCarrier *w)
{
    hashtable_iterator_t it;
    size_t total_len = 0;

    assert(w);
    assert(w->friends);

    friends_iterate(w->friends, &it);
    while(friends_iterator_has_next(&it)) {
        FriendInfo *fi;

        if (friends_iterator_next(&it, &fi) == 1) {
            size_t label_len = strlen(fi->info.label);
            if (label_len)
                total_len += sizeof(uint32_t) + label_len + 1;

            deref(fi);
        }
    }

    return total_len;
}

static void get_extra_savedata(ElaCarrier *w, void *data, size_t len)
{
    hashtable_iterator_t it;
    uint8_t *pos = (uint8_t *)data;

    assert(w);
    assert(w->friends);
    assert(data);

    friends_iterate(w->friends, &it);
    while(friends_iterator_has_next(&it) && len > 0) {
        FriendInfo *fi;

        if (friends_iterator_next(&it, &fi) == 1) {
            uint32_t nid;
            size_t label_len = strlen(fi->info.label);
            if (label_len) {
                if (len < (sizeof(uint32_t) + label_len + 1))
                    break;

                nid = htonl(fi->friend_number);
                memcpy(pos, &nid, sizeof(uint32_t));
                pos += sizeof(uint32_t);
                memcpy(pos, fi->info.label, label_len + 1);
                pos += (label_len + 1);

                len -= (sizeof(uint32_t) + label_len + 1);
            }

            deref(fi);
        }
    }

    return;
}

#ifdef _MSC_VER
// For Windows socket API not compatible with POSIX: size_t vs. int
#pragma warning(push)
#pragma warning(disable: 4267)
#endif

static int store_persistence_data(ElaCarrier *w)
{
    uint8_t *buf;
    uint8_t *pos;
    char *journal_filename;
    char *filename;
    uint32_t val;
    int fd;
    int rc;

    size_t dht_data_len;
    size_t extra_data_len;
    size_t total_len;

    assert(w);

    dht_data_len = dht_get_savedata_size(&w->dht);
    extra_data_len = get_extra_savedata_size(w);
    total_len = 256 + ROUND256(dht_data_len) + ROUND256(extra_data_len);

    buf = (uint8_t *)calloc(total_len, 1);
    if (!buf)
        return ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY);

    pos = buf;
    val = htonl(PERSISTENCE_MAGIC);
    memcpy(pos, &val, sizeof(uint32_t));

    pos += sizeof(uint32_t);
    val = htonl(PERSISTENCE_REVISION);
    memcpy(pos, &val, sizeof(uint32_t));

    pos += sizeof(uint32_t);
    val = htonl((uint32_t)dht_data_len);
    memcpy(pos, &val, sizeof(uint32_t));

    pos += sizeof(uint32_t);
    val = htonl((uint32_t)extra_data_len);
    memcpy(pos, &val, sizeof(uint32_t));

    pos = buf + 256;
    dht_get_savedata(&w->dht, pos);
    pos += ROUND256(dht_data_len);
    get_extra_savedata(w, pos, ROUND256(extra_data_len));

    pos = buf + 256;
    sha256(pos, total_len - 256, buf + (sizeof(uint32_t) * 4), SHA256_BYTES);

    rc = mkdirs(w->pref.data_location, S_IRWXU);
    if (rc < 0) {
        free(buf);
        return ELA_SYS_ERROR(errno);
    }

    filename = (char *)alloca(strlen(w->pref.data_location) + strlen(data_filename) + 4);
    sprintf(filename, "%s/%s", w->pref.data_location, data_filename);
    journal_filename = (char *)alloca(strlen(w->pref.data_location) + strlen(data_filename) + 16);
    sprintf(journal_filename, "%s/%s.journal", w->pref.data_location, data_filename);

    fd = open(journal_filename, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        free(buf);
        return ELA_SYS_ERROR(errno);
    }

    if (write(fd, buf, total_len) != total_len) {
        close(fd);
        remove(journal_filename);
        return ELA_SYS_ERROR(errno);
    }

    if (fsync(fd) < 0) {
        close(fd);
        remove(journal_filename);
        return ELA_SYS_ERROR(errno);
    }

    close(fd);
    free(buf);

    remove(filename);
    rename(journal_filename, filename);

    return 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static void ela_destroy(void *argv)
{
    ElaCarrier *w = (ElaCarrier *)argv;

    if (w->pref.data_location)
        free(w->pref.data_location);

    if (w->pref.dht_bootstraps)
        free(w->pref.dht_bootstraps);

    if (w->pref.hive_bootstraps)
        free(w->pref.hive_bootstraps);

    if (w->tassembly_irsps)
        deref(w->tassembly_irsps);

    if (w->tassembly_ireqs)
        deref(w->tassembly_ireqs);

    if (w->tcallbacks)
        deref(w->tcallbacks);

    if (w->thistory)
        deref(w->thistory);

    if (w->friends)
        deref(w->friends);

    if (w->friend_events)
        deref(w->friend_events);

    if (w->dstorectx)
        deref(w->dstorectx);

    pthread_mutex_destroy(&w->ext_mutex);

    dht_kill(&w->dht);
}

static void handle_offline_msg(EventBase *event, ElaCarrier *w)
{
    OfflineMsgEvent *ev = (OfflineMsgEvent *)event;

    ElaCP *cp;

    cp = elacp_decode(ev->content, ev->len);
    if (!cp) {
        vlogE("Carrier: Invalid DHT message, dropped.");
        return;
    }

    if (elacp_get_type(cp) != ELACP_TYPE_MESSAGE) {
        vlogE("Carrier: Unknown DHT message, dropped.");
        return;
    }

    if (!elacp_get_extension(cp) && ela_is_friend(w, ev->from) &&
        w->callbacks.friend_message)
        w->callbacks.friend_message(w, ev->from, elacp_get_raw_data(cp),
                                    elacp_get_raw_data_length(cp), true,
                                    w->context);
}

static void notify_offline_msg(ElaCarrier *w, const char *from,
                               const uint8_t *msg, size_t len)
{
    OfflineMsgEvent *event;

    assert(w);
    assert(from && *from);
    assert(msg);
    assert(len);

    event = rc_zalloc(sizeof(OfflineMsgEvent) + len, NULL);
    if (event) {
        strcpy(event->from, from);
        memcpy(event->content, msg, len);
        event->len = len;
        event->base.le.data = event;
        event->base.handle = handle_offline_msg;
        list_push_tail(w->friend_events, &event->base.le);
        deref(event);
    }
}

ElaCarrier *ela_new(const ElaOptions *opts, ElaCallbacks *callbacks,
                    void *context)
{
    ElaCarrier *w;
    persistence_data data;
    int rc;
    size_t i;

    if (!opts) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    if (!opts->persistent_location || !*opts->persistent_location) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    ela_log_init(opts->log_level, opts->log_file, opts->log_printer);

    w = (ElaCarrier *)rc_zalloc(sizeof(ElaCarrier), ela_destroy);
    if (!w) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    w->pref.udp_enabled = opts->udp_enabled;
    w->pref.data_location = strdup(opts->persistent_location);
    w->pref.dht_bootstraps_size = opts->bootstraps_size;

    w->pref.dht_bootstraps = (DhtBootstrapNodeBuf *)calloc(1, sizeof(DhtBootstrapNodeBuf)
                         * opts->bootstraps_size);
    if (!w->pref.dht_bootstraps) {
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    for (i = 0; i < opts->bootstraps_size; i++) {
        BootstrapNode *b = &opts->bootstraps[i];
        DhtBootstrapNodeBuf *bi = &w->pref.dht_bootstraps[i];
        char *endptr = "";
        ssize_t len;

        if (b->ipv4 && strlen(b->ipv4) > MAX_IPV4_ADDRESS_LEN) {
            vlogE("Carrier: DHT bootstrap ipv4 address (%s) too long", b->ipv4);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        if (b->ipv6 && strlen(b->ipv6) > MAX_IPV6_ADDRESS_LEN) {
            vlogE("Carrier: DHT bootstrap ipv4 address (%s) too long", b->ipv6);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        if (!b->ipv4 && !b->ipv6) {
            vlogE("Carrier: DHT bootstrap ipv4 and ipv6 address both empty");
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        if (b->ipv4)
            strcpy(bi->ipv4, b->ipv4);
        if (b->ipv6)
            strcpy(bi->ipv6, b->ipv6);

        bi->port = b->port ? (int)strtol(b->port, &endptr, 10) : DHT_BOOTSTRAP_DEFAULT_PORT;
        if (bi->port < 1 || bi->port > 65535 || *endptr) {
            vlogE("Carrier: Invalid DHT bootstrap port value (%s)", b->port);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        len = base58_decode(b->public_key, strlen(b->public_key), bi->public_key,
                            sizeof(bi->public_key));
        if (len != DHT_PUBLIC_KEY_SIZE) {
            vlogE("Carrier: Invalid DHT bootstrap public key (%s)", b->public_key);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }
    }

    w->pref.hive_bootstraps_size = opts->hive_bootstraps_size;
    if (w->pref.hive_bootstraps_size > 0) {
        w->pref.hive_bootstraps = (HiveBootstrapNodeBuf *)calloc(1,
                            sizeof(HiveBootstrapNodeBuf) * opts->hive_bootstraps_size);
        if (!w->pref.hive_bootstraps) {
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
            return NULL;
        }
    } else {
        w->pref.hive_bootstraps = NULL;
    }

    for (i = 0; i < opts->hive_bootstraps_size; i++) {
        HiveBootstrapNode *b = &opts->hive_bootstraps[i];
        HiveBootstrapNodeBuf *bi = &w->pref.hive_bootstraps[i];
        char *endptr = "";

        if (b->ipv4 && strlen(b->ipv4) > MAX_IPV4_ADDRESS_LEN) {
            vlogE("Carrier: Hive bootstrap ipv4 address (%s) too long", b->ipv4);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        if (b->ipv6 && strlen(b->ipv6) > MAX_IPV6_ADDRESS_LEN) {
            vlogE("Carrier: Hive bootstrap ipv4 address (%s) too long", b->ipv6);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        if (!b->ipv4 && !b->ipv6) {
            vlogE("Carrier: Hive bootstrap ipv4 and ipv6 address both empty");
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }

        if (b->ipv4)
            strcpy(bi->ipv4, b->ipv4);
        if (b->ipv6)
            strcpy(bi->ipv6, b->ipv6);

        bi->port = b->port ? (int)strtol(b->port, &endptr, 10) : HIVE_BOOTSTRAP_DEFAULT_PORT;
        if (bi->port < 1 || bi->port > 65535 || *endptr) {
            vlogE("Carrier: Invalid Hive bootstrap port value (%s)", b->port);
            deref(w);
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
            return NULL;
        }
    }

    if (w->pref.hive_bootstraps_size > 0) {
        w->dstorectx = dstore_wrapper_create(w, notify_offline_msg);
        if (!w->dstorectx) {
            vlogE("Carrier: Creating dstore warpper error (%s)", ela_get_error());
            deref(w);
            return NULL;
        }
    }

    memset(&data, 0, sizeof(data));
    load_persistence_data(opts->persistent_location, &data);

    rc = dht_new(data.dht_savedata, data.dht_savedata_len, w->pref.udp_enabled, &w->dht);
    if (rc < 0) {
        free_persistence_data(&data);
        deref(w);
        ela_set_error(rc);
        return NULL;
    }

    w->friends = friends_create(31);
    if (!w->friends) {
        free_persistence_data(&data);
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    w->friend_events = list_create(1, NULL);
    if (!w->friend_events) {
        free_persistence_data(&data);
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    w->tcallbacks = transacted_callbacks_create(31);
    if (!w->tcallbacks) {
        free_persistence_data(&data);
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    w->thistory = transaction_history_create(31);
    if (!w->thistory) {
        free_persistence_data(&data);
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    w->tassembly_ireqs = tassemblies_create(17);
    if (!w->tassembly_ireqs) {
        free_persistence_data(&data);
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    w->tassembly_irsps = tassemblies_create(17);
    if (!w->tassembly_irsps) {
        free_persistence_data(&data);
        deref(w);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return NULL;
    }

    rc = dht_get_self_info(&w->dht, get_self_info_cb, w);
    if (rc < 0) {
        free_persistence_data(&data);
        deref(w);
        ela_set_error(rc);
        return NULL;
    }

    rc = dht_get_friends(&w->dht, friends_iterate_cb, w);
    if (rc < 0) {
        free_persistence_data(&data);
        deref(w);
        ela_set_error(rc);
        return NULL;
    }

    rc = pthread_mutex_init(&w->ext_mutex, NULL);
    if (rc) {
        free_persistence_data(&data);
        deref(w);
        ela_set_error(ELA_SYS_ERROR(rc));
        return NULL;
    }

    apply_extra_data(w, data.extra_savedata, data.extra_savedata_len);
    free_persistence_data(&data);

    store_persistence_data(w);

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

    if (w->dstorectx)
        dstore_wrapper_kill(w->dstorectx);

    if (w->running) {
        w->quit = 1;

        if (!pthread_equal(pthread_self(), w->main_thread))
            while(!w->quit) usleep(5000);
    }

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
    hashtable_iterator_t it;

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

    if (connected && w->dstorectx)
        dstore_enqueue_pollmsg(w->dstorectx);
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
    bool changed = false;

    assert(friend_number != UINT32_MAX);
    assert(desc);

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number %u, friend description message "
              "dropped.", friend_number);
        return;
    }

    if (length == 0) {
        vlogW("Carrier: Empty description message from friend "
              "number %u, dropped.", friend_number);
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
        vlogE("Carrier: Unknown friend number %u, connection status message "
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
        vlogE("Carrier: Unknown friend number (%u), friend presence message "
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

    if (*name)
        strcpy(ui.name, name);
    if (*descr)
        strcpy(ui.description, descr);

    if (w->callbacks.friend_request)
        w->callbacks.friend_request(w, ui.userid, &ui, hello, w->context);

    elacp_free(cp);
}

static void handle_friend_added_event(EventBase *event, ElaCarrier *w)
{
    FriendEvent *ev = (FriendEvent *)event;

    if (w->callbacks.friend_added)
        w->callbacks.friend_added(w, &ev->fi, w->context);
}

static void notify_friend_added(ElaCarrier *w, ElaFriendInfo *fi)
{
    FriendEvent *event;

    assert(w);
    assert(fi);

    store_persistence_data(w);

    event = (FriendEvent *)rc_alloc(sizeof(FriendEvent), NULL);
    if (event) {
        memcpy(&event->fi, fi, sizeof(*fi));
        event->base.le.data = event;
        event->base.handle = handle_friend_added_event;
        list_push_tail(w->friend_events, &event->base.le);
        deref(event);
    }
}

static void handle_friend_removed_event(EventBase *event, ElaCarrier *w)
{
    FriendEvent *ev = (FriendEvent *)event;

    if (ev->fi.status == ElaConnectionStatus_Connected &&
        w->callbacks.friend_connection)
        w->callbacks.friend_connection(w, ev->fi.user_info.userid,
                                       ElaConnectionStatus_Disconnected,
                                       w->context);

    if (w->callbacks.friend_removed)
        w->callbacks.friend_removed(w, ev->fi.user_info.userid, w->context);
}

static void notify_friend_removed(ElaCarrier *w, ElaFriendInfo *fi)
{
    FriendEvent *event;

    assert(w);
    assert(fi);

    store_persistence_data(w);

    event = (FriendEvent *)rc_alloc(sizeof(FriendEvent), NULL);
    if (event) {
        memcpy(&event->fi, fi, sizeof(*fi));
        event->base.le.data = event;
        event->base.handle = handle_friend_removed_event;
        list_push_tail(w->friend_events, &event->base.le);
        deref(event);
    }
}

static void do_friend_events(ElaCarrier *w)
{
    list_t *events = w->friend_events;
    list_iterator_t it;

redo_events:
    list_iterate(events, &it);
    while (list_iterator_has_next(&it)) {
        EventBase *event;
        int rc;

        rc = list_iterator_next(&it, (void **)&event);
        if (rc == 0)
            break;

        if (rc == -1)
            goto redo_events;

        event->handle(event, w);
        list_iterator_remove(&it);

        deref(event);
    }
}

static void do_tassemblies_expire(hashtable_t *tassemblies)
{
    hashtable_iterator_t it;
    struct timeval now;

    gettimeofday(&now, NULL);

    tassemblies_iterate(tassemblies, &it);
    while(tassemblies_iterator_has_next(&it)) {
        TransactedAssembly *item;
        int rc;

        rc = tassemblies_iterator_next(&it, &item);
        if (rc <= 0)
            break;

        if (timercmp(&now, &item->expire_time, >))
            tassemblies_iterator_remove(&it);

        deref(item);
    }
}

static
void transacted_callback_expire(ElaCarrier *w, TransactedCallback *callback)
{
    char friendid[ELA_MAX_ID_LEN + 1];
    ElaFriendInviteResponseCallback *callback_func;
    FriendInfo *fi;

    fi = friends_get(w->friends, callback->friend_number);
    if (!fi)
        return;

    strcpy(friendid, fi->info.user_info.userid);
    deref(fi);

    callback_func = (ElaFriendInviteResponseCallback *)callback->callback_func;
    assert(callback_func);

    callback_func(w, friendid, callback->bundle, ELA_STATUS_TIMEOUT, "timeout",
                  NULL, 0, callback->callback_context);
}

static void do_transacted_callabcks_check(ElaCarrier *w)
{
    hashtable_iterator_t it;
    struct timeval now;

    gettimeofday(&now, NULL);

    transacted_callbacks_iterate(w->tcallbacks, &it);
    while(transacted_callbacks_iterator_has_next(&it)) {
        TransactedCallback *tcb;
        int rc;

        rc = transacted_callbacks_iterator_next(&it, &tcb);
        if (rc <= 0)
            break;

        if (timercmp(&now, &tcb->expire_time, >)) {
            hashtable_iterator_remove(&it);
            transacted_callback_expire(w, tcb);
        }

        deref(tcb);
    }
}

static
void handle_friend_message(ElaCarrier *w, uint32_t friend_number, ElaCP *cp)
{
    FriendInfo *fi;
    char friendid[ELA_MAX_ID_LEN + 1];
    const char *name;
    const void *msg;
    size_t len;

    assert(w);
    assert(friend_number != UINT32_MAX);
    assert(cp);
    assert(elacp_get_type(cp) == ELACP_TYPE_MESSAGE);

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number %u, friend message dropped.",
              friend_number);
        return;
    }

    strcpy(friendid, fi->info.user_info.userid);
    deref(fi);

    name = elacp_get_extension(cp);
    msg  = elacp_get_raw_data(cp);
    len  = elacp_get_raw_data_length(cp);

    if (w->callbacks.friend_message && !name)
        w->callbacks.friend_message(w, friendid, msg, len, false, w->context);
}

static
void handle_invite_request(ElaCarrier *w, uint32_t friend_number, ElaCP *cp)
{
    FriendInfo *fi;
    char friendid[ELA_MAX_ID_LEN + 1];
    const char *name;
    const void *data;
    const char *bundle;
    size_t bundle_len;
    size_t len;
    int64_t tid;
    size_t totalsz;
    bool need_add = false;
    char from[ELA_MAX_ID_LEN + ELA_MAX_EXTENSION_NAME_LEN + 4];
    TransactedAssembly *ireq = NULL;

    assert(w);
    assert(friend_number != UINT32_MAX);
    assert(cp);
    assert(elacp_get_type(cp) == ELACP_TYPE_INVITE_REQUEST);

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number %u, invite request dropped.",
              friend_number);
        return;
    }

    strcpy(friendid, fi->info.user_info.userid);
    deref(fi);

    bundle = elacp_get_bundle(cp);
    bundle_len = bundle ? strlen(bundle) + 1 : 0;
    name = elacp_get_extension(cp);
    data = elacp_get_raw_data(cp);
    len  = elacp_get_raw_data_length(cp);
    tid  = elacp_get_tid(cp);
    totalsz = elacp_get_totalsz(cp);

    ireq = tassemblies_get(w->tassembly_ireqs, tid);
    if (!ireq) {
        struct timeval now, expire_interval;

        if (!totalsz || totalsz > ELA_MAX_INVITE_DATA_LEN) {
            vlogW("Carrier: Received invite request fragment with invalid "
                  "totalsz %z, dropped.", totalsz);
            return;
        }

        ireq = (TransactedAssembly *)rc_zalloc(sizeof(*ireq) + totalsz + bundle_len, NULL);
        if (!ireq) {
            vlogW("Carrier: Out of memory, invite request fragment dropped.");
            return;
        }

        strcpy(ireq->ext, name ? name : "");
        strcpy(ireq->friendid, friendid);
        ireq->tid = tid;
        ireq->data_len = totalsz;
        ireq->data_off = 0;
        ireq->data = (uint8_t*)(ireq + 1);
        if (bundle_len > 0) {
            ireq->bundle = (char *)(ireq->data + totalsz);
            strcpy(ireq->bundle, bundle);
        } else {
            ireq->bundle = NULL;
        }

        gettimeofday(&now, NULL);
        expire_interval.tv_sec = TASSEMBLY_TIMEOUT;
        expire_interval.tv_usec = 0;
        timeradd(&now, &expire_interval, &ireq->expire_time);

        need_add = true;  //Ready to put into tassembly hashtable.
    }

    if ((name && strcmp(ireq->ext, name)) ||
        strcmp(ireq->friendid, friendid) || !len || len > INVITE_DATA_UNIT ||
        ireq->data_off + len < len || ireq->data_off + len > ireq->data_len) {
        vlogE("Carrier: Inavlid invite request fragment (or HACKED), dropped.");
        deref(ireq);
        return;
    }

    memcpy(ireq->data + ireq->data_off, data, len);
    ireq->data_off += len;

    if (ireq->data_off == ireq->data_len) {
        strcpy(from, friendid);
        if (name) {
            strcat(from, ":");
            strcat(from, name);
        }
        transaction_history_put_invite(w->thistory, from, tid);

        if (name) {
            if (strcmp(name, "session") == 0) {
                SessionExtension *ext = (SessionExtension *)w->session;
                if (ext && ext->friend_invite_cb)
                    ext->friend_invite_cb(w, friendid, ireq->bundle, (const void*)ireq->data,
                                          ireq->data_len, ext);
            }
        } else {
            if (w->callbacks.friend_invite)
                w->callbacks.friend_invite(w, friendid, ireq->bundle, ireq->data, ireq->data_len,
                                           w->context);
        }

        if (!need_add)
            tassemblies_remove(w->tassembly_ireqs, tid);
        else
            need_add = false;
    }

    if (need_add)
        tassemblies_put(w->tassembly_ireqs, ireq);
    deref(ireq);
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
    size_t totalsz;
    int status;
    const char *bundle;
    const char *name;
    const void *data = NULL;
    const char *reason = NULL;
    size_t bundle_len;
    size_t reason_len = 0;
    size_t data_len = 0;
    bool need_add = false;
    TransactedAssembly *irsp = NULL;

    assert(w);
    assert(friend_number != UINT32_MAX);
    assert(cp);
    assert(elacp_get_type(cp) == ELACP_TYPE_INVITE_RESPONSE);

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number %u, invite response dropped.",
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

    bundle = elacp_get_bundle(cp);
    bundle_len = bundle ? strlen(bundle) + 1 : 0;
    name = elacp_get_extension(cp);
    totalsz = elacp_get_totalsz(cp);
    status = elacp_get_status(cp);
    if (status) {
        reason = elacp_get_reason(cp);
        reason_len = strlen(reason) + 1;
    } else {
        data = elacp_get_raw_data(cp);
        data_len = elacp_get_raw_data_length(cp);
    }

    irsp = tassemblies_get(w->tassembly_irsps, tid);
    if (!irsp) {
        struct timeval now, expire_interval;

        if (totalsz > ELA_MAX_INVITE_DATA_LEN) {
            vlogW("Carrier: Received overlong invite request fragment, "
                  "dropped.");
            deref(tcb);
            return;
        }

        irsp = (TransactedAssembly *)rc_zalloc(sizeof(*irsp) + totalsz +
                                            + bundle_len + reason_len, NULL);
        if (!irsp) {
            vlogW("Carrier: Out of memory, invite response fragment dropped.");
            deref(tcb);
            return;
        }

        strcpy(irsp->ext, name ? name : "");
        strcpy(irsp->friendid, friendid);
        irsp->tid = tid;
        irsp->data_len = totalsz;
        irsp->data_off = 0;
        irsp->data = totalsz ? (uint8_t *)(irsp + 1) : NULL;

        if (bundle_len > 0) {
            irsp->bundle = (char *)(irsp + 1) + totalsz;
            strcpy(irsp->bundle, bundle);
        }

        if (reason_len > 0) {
            irsp->reason = (char *)(irsp + 1) + totalsz + bundle_len;
            strcpy(irsp->reason, reason);
        }

        gettimeofday(&now, NULL);
        expire_interval.tv_sec = TASSEMBLY_TIMEOUT;
        expire_interval.tv_usec = 0;
        timeradd(&now, &expire_interval, &irsp->expire_time);

        need_add = true;
    }

    if ((name && strcmp(irsp->ext, name)) || strcmp(irsp->friendid, friendid) ||
        data_len > INVITE_DATA_UNIT || irsp->data_off + data_len < data_len ||
        irsp->data_off + data_len > irsp->data_len) {
        vlogE("Carrier: Inavlid invite response fragment (or HACKED), dropped.");
        deref(irsp);
        deref(tcb);
        return;
    }

    if (data) {
        memcpy(irsp->data + irsp->data_off, data, data_len);
        irsp->data_off += data_len;
    }

    if (irsp->data_off == irsp->data_len) {
        callback_func = (ElaFriendInviteResponseCallback *)tcb->callback_func;
        callback_ctxt = tcb->callback_context;
        assert(callback_func);

        transacted_callbacks_remove(w->tcallbacks, tid);

        callback_func(w, friendid, irsp->bundle, status, reason, irsp->data, irsp->data_len,
                      callback_ctxt);

        if (!need_add)
            tassemblies_remove(w->tassembly_irsps, tid);
        else
            need_add = false;
    }

    if (need_add)
        tassemblies_put(w->tassembly_irsps, irsp);

    deref(irsp);
    deref(tcb);
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

static
void notify_group_invite_cb(uint32_t friend_number, const uint8_t *cookie,
                            size_t len, void *user_data)
{
    FriendInfo *fi;
    ElaCarrier *w = (ElaCarrier *)user_data;

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        vlogE("Carrier: Unknown friend number %u, group invite dropped.",
              friend_number);
        return;
    }

    if (w->callbacks.group_invite) {
        char friendid[ELA_MAX_ID_LEN + 1];

        strcpy(friendid, fi->info.user_info.userid);
        w->callbacks.group_invite(w, friendid, cookie, len, w->context);
    }

    deref(fi);
}

static
int get_groupid_by_number(ElaCarrier *w, uint32_t group_number,
                          char *groupid_buf, size_t length)
{
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
    size_t textlen = length;
    int rc;

    assert(length >= ELA_MAX_ID_LEN + 1);

    rc = dht_group_get_public_key(&w->dht, group_number, public_key);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    base58_encode(public_key, sizeof(public_key), groupid_buf, &textlen);

    return 0;
}

static
int get_peerid_by_number(ElaCarrier *w, uint32_t group_number,
                         uint32_t peer_number, char *peerid_buf, size_t length)
{
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
    size_t textlen = length;
    int rc;

    assert(length >= ELA_MAX_ID_LEN + 1);

    rc = dht_group_get_peer_public_key(&w->dht, group_number, peer_number,
                                       public_key);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    base58_encode(public_key, sizeof(public_key), peerid_buf, &textlen);

    return 0;
}

static
void notify_group_connected_cb(uint32_t group_number, void *user_data)
{
    ElaCarrier *w = (ElaCarrier *)user_data;
    char groupid[ELA_MAX_ID_LEN + 1];
    int rc;

    rc = get_groupid_by_number(w, group_number, groupid, sizeof(groupid));
    if (rc < 0) {
        vlogE("Carrier: Unknown group number %u, group connection dropped.",
              group_number);
        return;
    }

    if (w->callbacks.group_callbacks.group_connected)
        w->callbacks.group_callbacks.group_connected(w, groupid, w->context);
}

static
void notify_group_message_cb(uint32_t group_number, uint32_t peer_number,
                             const uint8_t *msg, size_t len, void *user_data)
{
    ElaCarrier *w = (ElaCarrier *)user_data;
    char groupid[ELA_MAX_ID_LEN + 1];
    char peerid[ELA_MAX_ID_LEN + 1];
    int rc;

    rc = get_groupid_by_number(w, group_number, groupid, sizeof(groupid));
    if (rc < 0) {
        vlogE("Carrier: Unknown group number %u, group message dropped.",
              group_number);
        return;
    }

    rc = get_peerid_by_number(w, group_number, peer_number, peerid,
                              sizeof(peerid));
    if (rc < 0) {
        vlogE("Carrier: Unknown peer number %u, group message dropped.",
              peer_number);
        return;
    }

    if (w->callbacks.group_callbacks.group_message)
        w->callbacks.group_callbacks.group_message(w, groupid, peerid,
                                                  msg, len, w->context);
}

static
void notify_group_title_cb(uint32_t group_number, uint32_t peer_number,
                           const uint8_t *title, size_t length, void *user_data)
{
    ElaCarrier *w = (ElaCarrier *)user_data;
    char groupid[ELA_MAX_ID_LEN + 1];
    char peerid[ELA_MAX_ID_LEN + 1];
    int rc;

    if (peer_number == UINT32_MAX) {
        vlogI("Carrier: Do not notify newly joined peer about the group name.");
        return;
    }

    rc = get_groupid_by_number(w, group_number, groupid, sizeof(groupid));
    if (rc < 0) {
        vlogE("Carrier: Unknown group number %u, group titile change event "
              "dropped.", group_number);
        return;
    }

    rc = get_peerid_by_number(w, group_number, peer_number, peerid,
                              sizeof(peerid));
    if (rc < 0) {
        vlogE("Carrier: Unknown peer number %u, group titile change event "
              "dropped.", peer_number);
        return;
    }

    if (w->callbacks.group_callbacks.group_title)
        w->callbacks.group_callbacks.group_title(w, groupid, peerid,
                                                length ?
                                                (const char *)title : "",
                                                w->context);
}

static
void notify_group_peer_name_cb(uint32_t group_number, uint32_t peer_number,
                               const uint8_t *name, size_t length,
                               void *user_data)
{
    ElaCarrier *w = (ElaCarrier *)user_data;
    char groupid[ELA_MAX_ID_LEN + 1];
    char peerid[ELA_MAX_ID_LEN + 1];
    int rc;

    rc = get_groupid_by_number(w, group_number, groupid, sizeof(groupid));
    if (rc < 0) {
        vlogE("Carrier: Unknown group number %u, group peer name change event "
              "dropped.", group_number);
        return;
    }

    rc = get_peerid_by_number(w, group_number, peer_number, peerid,
                              sizeof(peerid));
    if (rc < 0) {
        vlogE("Carrier: Unknown peer number %u, group peer name change event "
              "dropped.", peer_number);
        return;
    }

    if (w->callbacks.group_callbacks.peer_name)
        w->callbacks.group_callbacks.peer_name(w, groupid, peerid,
                                               length ? (char *)name : "",
                                               w->context);
}

static
void notify_group_peer_list_changed_cb(uint32_t group_number, void *user_data)
{
    ElaCarrier *w = (ElaCarrier *)user_data;
    char groupid[ELA_MAX_ID_LEN + 1];
    int rc;

    rc = get_groupid_by_number(w, group_number, groupid, sizeof(groupid));
    if (rc < 0) {
        vlogE("Carrier: Unknown group number %u, group titile change event "
              "dropped.", group_number);
        return;
    }

    if (w->callbacks.group_callbacks.peer_list_changed)
        w->callbacks.group_callbacks.peer_list_changed(w, groupid, w->context);
}

static void connect_to_bootstraps(ElaCarrier *w)
{
    int i;

    for (i = 0; i < w->pref.dht_bootstraps_size; i++) {
        DhtBootstrapNodeBuf *bi = &w->pref.dht_bootstraps[i];
        char id[ELA_MAX_ID_LEN + 1] = {0};
        size_t id_len = sizeof(id);
        int rc;

        base58_encode(bi->public_key, DHT_PUBLIC_KEY_SIZE, id, &id_len);
        rc = _dht_bootstrap(&w->dht, bi->ipv4, bi->ipv6, bi->port, bi->public_key);
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
    w->dht_callbacks.notify_group_invite = notify_group_invite_cb;
    w->dht_callbacks.notify_group_connected = notify_group_connected_cb;
    w->dht_callbacks.notify_group_message = notify_group_message_cb;
    w->dht_callbacks.notify_group_title = notify_group_title_cb;
    w->dht_callbacks.notify_group_peer_name = notify_group_peer_name_cb;
    w->dht_callbacks.notify_group_peer_list_changed = notify_group_peer_list_changed_cb;
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

        do_friend_events(w);
        do_tassemblies_expire(w->tassembly_ireqs);
        do_tassemblies_expire(w->tassembly_irsps);
        do_transacted_callabcks_check(w);

        if (idle_interval > 0)
            notify_idle(w);

        gettimeofday(&check, NULL);

        if (timercmp(&expire, &check, >)) {
            timersub(&expire, &check, &tmp);
            usleep(tmp.tv_usec);
        }

        dht_iterate(&w->dht, &w->dht_callbacks);
    }

    w->running = 0;

    store_persistence_data(w);

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

    store_persistence_data(w);

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
        if (strcmp(info->name, w->me.name)) {
            int rc = dht_self_set_name(&w->dht, (uint8_t *)info->name,
                                       strlen(info->name) + 1);
            if (rc) {
                elacp_free(cp);
                ela_set_error(rc);
                return -1;
            }
        }

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

        store_persistence_data(w);

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

    ela_set_error(0);
    return w->is_ready;
}

int ela_get_friends(ElaCarrier *w,
                    ElaFriendsIterateCallback *callback, void *context)
{
    hashtable_iterator_t it;

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

    if (!w || !friendid || !*friendid || !info) {
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

    if (!w || !friendid || !*friendid) {
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

    store_persistence_data(w);

    return 0;
}

bool ela_is_friend(ElaCarrier *w, const char *userid)
{
    uint32_t friend_number;
    int rc;

    if (!w || !userid || !*userid) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return false;
    }

    rc = get_friend_number(w, userid, &friend_number);
    if (rc < 0 || friend_number == UINT32_MAX) {
        ela_set_error(rc);
        return false;
    }

    ela_set_error(0);
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

    if (!w || !hello || !*hello || !address || !*address) {
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

    if (!w || !userid || !*userid) {
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
    int rc;

    if (!w || !friendid || !*friendid) {
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
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    if (!friends_exist(w->friends, friend_number)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    dht_friend_delete(&w->dht, friend_number);

    fi = friends_remove(w->friends, friend_number);
    assert(fi);

    notify_friend_removed(w, &fi->info);

    deref(fi);

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

int ela_send_friend_message(ElaCarrier *w, const char *to,
                            const void *msg, size_t len, bool *is_offline)
{
    char *addr, *userid, *ext_name;
    FriendInfo *fi;
    uint32_t friend_number;
    bool friend_online = false;
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

    fi = friends_get(w->friends, friend_number);
    if (!fi) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
        return -1;
    }

    friend_online = (fi->info.status == ElaConnectionStatus_Connected);
    deref(fi);

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

    if (friend_online) {
        rc = dht_friend_message(&w->dht, friend_number, data, data_len);
        if (!rc) {
            free(data);
            if (is_offline)
                *is_offline = false;

            return 0;
        }
    } else
        rc = ELA_DHT_ERROR(ELAERR_FRIEND_OFFLINE);

    if (w->dstorectx)
        rc = dstore_enqueue_offmsg(w->dstorectx, to, data, data_len);

    free(data);

    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    if (is_offline)
        *is_offline = true;

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

int ela_invite_friend(ElaCarrier *w, const char *to, const char *bundle,
                      const void *data, size_t len,
                      ElaFriendInviteResponseCallback *callback,
                      void *context)
{
    char *addr, *userid, *ext_name;
    uint32_t friend_number;
    ElaCP *cp;
    int rc;
    int64_t tid;
    int index = 0;
    int bundle_len = bundle ? strlen(bundle) : 0;
    char *pos = (char *)data;
    size_t send_len = 0;

    if (!w || (bundle && (!*bundle || bundle_len > ELA_MAX_BUNDLE_LEN))
           || !data || !len || len > ELA_MAX_INVITE_DATA_LEN || !callback) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!to || !(*to) || strlen(to) >
            (ELA_MAX_ID_LEN + sizeof(':') + ELA_MAX_EXTENSION_NAME_LEN)) {
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

    if (ext_name && (!(*ext_name) || strlen(ext_name) >
                                     ELA_MAX_EXTENSION_NAME_LEN)) {
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

    tid = generate_tid();

    do {
        uint8_t *_data;
        size_t _data_len;

        cp = elacp_create(ELACP_TYPE_INVITE_REQUEST, ext_name);
        if (!cp) {
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
            return -1;
        }

        elacp_set_tid(cp, &tid);
        ++index;

        if (len > 0) {
            if (bundle) {
                if (len + bundle_len <= INVITE_DATA_UNIT)
                    send_len = len;
                else
                    send_len = INVITE_DATA_UNIT - bundle_len;

                if (index == 1) {
                    elacp_set_bundle(cp, bundle);
                    bundle_len = 0;
                }
            } else {
                send_len = (len > INVITE_DATA_UNIT) ? INVITE_DATA_UNIT : len;
            }

            elacp_set_totalsz(cp, (index == 1 ? len : 0));
            elacp_set_raw_data(cp, pos, send_len);
            pos += send_len;
            len -= send_len;
        }

        _data = elacp_encode(cp, &_data_len);
        elacp_free(cp);

        if (!_data) {
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
            return -1;
        }

        if (len == 0) {
            TransactedCallback *tcb;
            tcb = (TransactedCallback *)rc_alloc(sizeof(TransactedCallback) +
                                    (bundle ? strlen(bundle) + 1 : 0), NULL);
            if (!tcb) {
                ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
                free(_data);
                return -1;
            }

            tcb->tid = tid;
            tcb->callback_func = callback;
            tcb->callback_context = context;
            if (bundle) {
                tcb->bundle = (char*)(tcb + 1);
                strcpy(tcb->bundle, bundle);
            } else {
                tcb->bundle = NULL;
            }

            transacted_callbacks_put(w->tcallbacks, tcb);
            deref(tcb);
        }

        rc = dht_friend_message(&w->dht, friend_number, _data, _data_len);
        free(_data);

        if (rc < 0) {
            if (len == 0)
                transacted_callbacks_remove(w->tcallbacks, tid);

            ela_set_error(rc);
            return -1;
        }
    } while (len > 0);

    return 0;
}

int ela_reply_friend_invite(ElaCarrier *w, const char *to, const char *bundle,
                            int status, const char *reason,
                            const void *data, size_t len)
{
    char *addr, *userid, *ext_name;
    uint32_t friend_number;
    int64_t tid;
    int index = 0;
    int bundle_len = bundle ? strlen(bundle) : 0;
    int reason_len = reason ? strlen(reason) : 0;
    char *pos = (char*)data;
    size_t send_len;
    ElaCP *cp;
    int rc;

    if (!w || (bundle && (!*bundle || bundle_len > ELA_MAX_BUNDLE_LEN))) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (status && (!reason || reason_len > ELA_MAX_INVITE_REPLY_REASON_LEN
            || data || len > 0)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!status && (reason || !data || !len || len > ELA_MAX_INVITE_DATA_LEN)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!to || !(*to) || strlen(to) >
            (ELA_MAX_ID_LEN + sizeof(':') + ELA_MAX_EXTENSION_NAME_LEN)) {
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

    if (ext_name && (!(*ext_name) || strlen(ext_name) >
                                     ELA_MAX_EXTENSION_NAME_LEN)) {
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

    tid = transaction_history_get_invite(w->thistory, to);
    if (tid == 0) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NO_MATCHED_REQUEST));
        return -1;
    }

    do {
        uint8_t *_data;
        size_t _data_len;

        cp = elacp_create(ELACP_TYPE_INVITE_RESPONSE, ext_name);
        if (!cp) {
            ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
            return -1;
        }

        elacp_set_tid(cp, &tid);
        elacp_set_status(cp, status);
        ++index;

        if (index == 1) {
            if (bundle)
                elacp_set_bundle(cp, bundle);

            if (status)
                elacp_set_reason(cp, reason);
        }

        if (!status && pos && len > 0) {
            if (bundle_len + len <= INVITE_DATA_UNIT)
                send_len = len;
            else {
                send_len = INVITE_DATA_UNIT;
                if (bundle_len > 0) {
                    send_len -= bundle_len;
                    bundle_len = 0;
                }
            }

            elacp_set_totalsz(cp, (index == 1 ? len : 0));
            elacp_set_raw_data(cp, pos, send_len);

            pos += send_len;
            len -= send_len;
        }

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
    } while (len > 0);

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

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
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

int ela_new_group(ElaCarrier *w, char *groupid, size_t length)
{
    uint32_t group_number;
    int rc;

    if (!w || !groupid || length <= ELA_MAX_ID_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = dht_group_new(&w->dht, &group_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    rc = get_groupid_by_number(w, group_number, groupid, length);
    if (rc < 0) {
        dht_group_leave(&w->dht, group_number);
        ela_set_error(rc);
        return -1;
    }

    vlogD("Carrier: Group %s created.", groupid);

    return 0;
}

static
int get_group_number(ElaCarrier *w, const char *groupid, uint32_t *group_number)
{
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
    ssize_t len;
    int rc;

    assert(w);
    assert(groupid);
    assert(group_number);

    len = base58_decode(groupid, strlen(groupid), public_key, sizeof(public_key));
    if (len != DHT_PUBLIC_KEY_SIZE) {
        vlogE("Carrier: groupid %s not base58 encoded.", groupid);
        return ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS);
    }

    rc = dht_group_number_by_public_key(&w->dht, public_key, group_number);
    if (rc < 0)
        return ELA_GENERAL_ERROR(ELAERR_NOT_EXIST);

    return rc;
}

int ela_leave_group(ElaCarrier *w, const char *groupid)
{
    uint32_t group_number;
    int rc;

    if (!w || !groupid || !*groupid) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_group_number(w, groupid, &group_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    rc = dht_group_leave(&w->dht, group_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    vlogD("Carrier: Leaved from Group %s", groupid);

    return 0;
}

int ela_group_invite(ElaCarrier *w, const char *groupid, const char *friendid)
{
    uint32_t friend_number;
    uint32_t group_number;
    int rc;

    if (!w || !groupid || !*groupid || !friendid || !*friendid) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_group_number(w, groupid, &group_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    rc = get_friend_number(w, friendid, &friend_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    rc = dht_group_invite(&w->dht, group_number, friend_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    vlogD("Carrier: Invite friend %s into group %s success", friendid,
          groupid);

    return 0;
}

int ela_group_join(ElaCarrier *w, const char *friendid, const void *cookie,
                   size_t cookie_len, char *groupid, size_t length)
{
    uint32_t friend_number;
    uint32_t group_number;
    int rc;

    if (!w || !friendid || !*friendid || !cookie || !cookie_len ||
        !groupid || length <= ELA_MAX_ID_LEN) {
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

    rc = dht_group_join(&w->dht, friend_number, (const uint8_t *)cookie,
                        cookie_len, &group_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    rc = get_groupid_by_number(w, group_number, groupid, length);
    if (rc < 0) {
        dht_group_leave(&w->dht, group_number);
        ela_set_error(rc);
        return -1;
    }

    vlogD("Carrier: Friend %s joined group %s success", friendid, groupid);

    return 0;
}

int ela_group_send_message(ElaCarrier *w, const char *groupid, const void *msg,
                           size_t length)
{
    uint32_t group_number;
    int rc;

    if (!w || !groupid || !*groupid || !msg || !length ||
        length > ELA_MAX_APP_MESSAGE_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_group_number(w, groupid, &group_number);
    if (rc < 0) {
        ela_set_error(rc);
        return  -1;
    }

    rc = dht_group_send_message(&w->dht, group_number, msg, length);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    return 0;
}

int ela_group_get_title(ElaCarrier *w, const char *groupid, char *title,
                        size_t length)
{
    uint32_t group_number;
    int rc;

    if (!w || !groupid || !*groupid || !title || !length) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_group_number(w, groupid, &group_number);
    if (rc < 0) {
        ela_set_error(rc);
        return  -1;
    }

    memset(title, 0, length);
    rc = dht_group_get_title(&w->dht, group_number, (uint8_t *)title, length);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    return 0;
}

int ela_group_set_title(ElaCarrier *w, const char *groupid, const char *title)
{
    uint32_t group_number;
    char buf[ELA_MAX_GROUP_TITLE_LEN + 1];
    int rc;

    if (!w || !groupid || !*groupid || !title || !*title ||
        strlen(title) > ELA_MAX_GROUP_TITLE_LEN) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_group_number(w, groupid, &group_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    rc = dht_group_get_title(&w->dht, group_number, (uint8_t *)buf, sizeof(buf));
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    if ((rc <= 1 && !*title) || (rc > 1 && strcmp(buf, title) == 0))
        return 0;

    rc = dht_group_set_title(&w->dht, group_number, (uint8_t *)title,
                             strlen(title) + 1);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    return 0;
}

int ela_group_get_peers(ElaCarrier *w, const char *groupid,
                        ElaGroupPeersIterateCallback *callback,
                        void *context)
{
    uint32_t group_number;
    uint32_t peer_count;
    uint32_t i;
    int rc;

    if (!w || !groupid || !*groupid || !callback) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_group_number(w, groupid, &group_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    rc = dht_group_peer_count(&w->dht, group_number, &peer_count);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    for (i = 0; i < peer_count; i++) {
        uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
        ElaGroupPeer peer;
        size_t text_sz = sizeof(peer.userid);
        char *peerid;

        rc = dht_group_get_peer_name(&w->dht, group_number, i, peer.name,
                                     sizeof(peer.name));
        if (rc < 0) {
            vlogW("Carrier: Get peer %lu name from group:%lu error.",
                  i, group_number);
            continue;
        } else if (rc == 0) {
            peer.name[0] = '\0';
        } else {
            //Dothing.
        }

        rc = dht_group_get_peer_public_key(&w->dht, group_number, i, public_key);
        if (rc < 0) {
            vlogW("Carrier: Get peer %lu public key from group %lu error.",
                  i, group_number);
            continue;
        }

        peerid = base58_encode(public_key, sizeof(public_key), peer.userid,
                               &text_sz);
        if (!peerid) {
            vlogW("Carrier: Convert public key to userid error");
            continue;
        }

        if (!callback(&peer, context))
            return 0;
    }

    rc = dht_group_offline_peer_count(&w->dht, group_number, &peer_count);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    for (i = 0; i < peer_count; i++) {
        uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
        ElaGroupPeer peer;
        size_t text_sz = sizeof(peer.userid);
        char *peerid;

        rc = dht_group_get_offline_peer_name(&w->dht, group_number, i, peer.name,
                                             sizeof(peer.name));
        if (rc < 0) {
            vlogW("Carrier: Get peer %lu name from group:%lu error.",
                  i, group_number);
            continue;
        } else if (rc == 0) {
            peer.name[0] = '\0';
        } else {
            //Dothing.
        }

        rc = dht_group_get_offline_peer_public_key(&w->dht, group_number, i, public_key);
        if (rc < 0) {
            vlogW("Carrier: Get peer %lu public key from group %lu error.",
                  i, group_number);
            continue;
        }

        peerid = base58_encode(public_key, sizeof(public_key), peer.userid,
                               &text_sz);
        if (!peerid) {
            vlogW("Carrier: Convert public key to userid error");
            continue;
        }

        if (!callback(&peer, context))
            return 0;
    }

    callback(NULL, context);
    return 0;
}

int ela_group_get_peer(ElaCarrier *w, const char *groupid,
                       const char *peerid, ElaGroupPeer *peer)
{
    uint8_t peerpk[DHT_PUBLIC_KEY_SIZE];
    uint32_t group_number;
    uint32_t peer_count;
    uint32_t i;
    int rc;

    if (!w || !groupid || !*groupid || !peerid || !*peerid || !peer) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    rc = (int)base58_decode(peerid, strlen(peerid), peerpk, sizeof(peerpk));
    if (rc != sizeof(peerpk)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    rc = get_group_number(w, groupid, &group_number);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    rc = dht_group_peer_count(&w->dht, group_number, &peer_count);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    for (i = 0; i < peer_count; i++) {
        uint8_t public_key[DHT_PUBLIC_KEY_SIZE];

        rc = dht_group_get_peer_public_key(&w->dht, group_number, i, public_key);
        if (rc < 0) {
            vlogW("Carrier: Get peer %lu name from group:%lu error.",
                  i, group_number);
            continue;
        }

        if (memcmp(peerpk, public_key, sizeof(peerpk)) == 0) {
            memset(peer->name, 0, sizeof(peer->name));
            rc = dht_group_get_peer_name(&w->dht, group_number, i, peer->name,
                                         sizeof(peer->name));
            if (rc < 0) {
                vlogE("Carrier: Get peer %lu name from group:%lu error.", i,
                      group_number);
                ela_set_error(rc);
                return -1;
            }

            strcpy(peer->userid, peerid);

            return 0;
        }
    }

    rc = dht_group_offline_peer_count(&w->dht, group_number, &peer_count);
    if (rc < 0) {
        ela_set_error(rc);
        return -1;
    }

    for (i = 0; i < peer_count; i++) {
        uint8_t public_key[DHT_PUBLIC_KEY_SIZE];

        rc = dht_group_get_offline_peer_public_key(&w->dht, group_number, i, public_key);
        if (rc < 0) {
            vlogW("Carrier: Get peer %lu name from group:%lu error.",
                  i, group_number);
            continue;
        }

        if (memcmp(peerpk, public_key, sizeof(peerpk)) == 0) {
            memset(peer->name, 0, sizeof(peer->name));
            rc = dht_group_get_offline_peer_name(&w->dht, group_number, i, peer->name,
                                                 sizeof(peer->name));
            if (rc < 0) {
                vlogE("Carrier: Get peer %lu name from group:%lu error.", i,
                      group_number);
                ela_set_error(rc);
                return -1;
            }

            strcpy(peer->userid, peerid);

            return 0;
        }
    }

    vlogE("Carrier: Can not find peer (%s) in group (%lu)", peerid, group_number);
    ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_EXIST));
    return -1;
}

int ela_get_groups(ElaCarrier *w, ElaIterateGroupCallback *callback,
                   void *context)
{
    uint32_t group_count;
    uint32_t *group_number_list;
    uint32_t i;

    if (!w || !callback) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!w->is_ready) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_NOT_READY));
        return -1;
    }

    group_count = dht_get_group_count(&w->dht);
    if (!group_count) {
        callback(NULL, context);
        return 0;
    }

    group_number_list = (uint32_t *)alloca(sizeof(uint32_t) * group_count);
    dht_get_group_list(&w->dht, group_number_list);

    for (i = 0; i < group_count; i++) {
        char groupid[ELA_MAX_ID_LEN + 1];
        int rc;

        rc = get_groupid_by_number(w, group_number_list[i], groupid,
                                   sizeof(groupid));
        if (rc < 0)
            continue;

        if (!callback(groupid, context))
            return 0;
    }

    callback(NULL, context);
    return 0;
}


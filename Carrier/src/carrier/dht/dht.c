#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>

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

#define ELAF_DHT  6

#define ELA_MK_ERROR(facility, code)  (0x80000000 | ((facility) << 24) | \
                    ((((code) & 0x80000000) >> 8) | ((code) & 0x7FFFFFFF)))

#define DHT_ERROR(api_error, error) ELA_MK_ERROR(ELAF_DHT, ((api_error) << 16 | error))

const char *data_filename = "dhtdata";

enum {
    ERR_NEW_DHT = 1,
    ERR_SELF_SET_NAME = 2,
    ERR_SELF_SET_STATUS_MESSAGE = 3,
    ERR_FRIEND_QUERY = 4,
    ERR_FRIEND_BY_PUBLIC_KEY = 5,
    ERR_FRIEND_GET_PUBLIC_KEY = 6,
    ERR_FRIEND_ADD = 7,
    ERR_FRIEND_SEND_MESSAGE = 8,
    ERR_FRIEND_DELETE = 9,
    ERR_BOOTSTRAP = 10
};

struct DHT {
    Tox *tox;
    const char *data_dir;
};

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
    vlog(_level, buf);
}

int dht_new(const char *data_location, bool udp_enabled, DHT *dht)
{
    char *filename;
    FILE *fp;
    uint8_t *data;
    size_t datalen;
    struct Tox_Options options;
    TOX_ERR_NEW error;
    Tox *tox;

    assert(data_location);
    assert(dht);

    filename = (char *)alloca(strlen(data_location) + strlen(data_filename) + 4);
    sprintf(filename, "%s/%s", data_location, data_filename);

    fp = fopen(filename, "r");
    if (!fp) {
        data = NULL;
        datalen = 0;
    } else {
        fseek(fp, 0, SEEK_END);
        datalen = ftell(fp);
        rewind(fp);

        data = (uint8_t *)alloca(datalen);

        if (fread(data, sizeof(uint8_t), datalen, fp) != datalen) {
            vlogE("DHT: Read save data file error.");
            fclose(fp);
            //TODO: error number.
            return -1;
        }
    }
    fclose(fp);

    tox_options_default(&options);
    options.local_discovery_enabled = true;
    options.ipv6_enabled = false;
    options.udp_enabled = udp_enabled;
    options.log_callback = log_cb;

    if (data) {
        options.savedata_type = TOX_SAVEDATA_TYPE_TOX_SAVE;
        options.savedata_data = data;
        options.savedata_length = datalen;
    } else {
        options.savedata_type = TOX_SAVEDATA_TYPE_NONE;
        options.savedata_data = NULL;
        options.savedata_length = 0;
    }

    tox = tox_new(&options, &error);
    if (!tox) {
        vlogE("DHT: new dht error (%d).", error);
        return DHT_ERROR(ERR_NEW_DHT, error);
    }

    tox_callback_self_connection_status(tox, notify_connection_cb);
    tox_callback_friend_status_message(tox, notify_friend_status_message_cb);
    tox_callback_friend_connection_status(tox, notify_friend_connection_cb);
    tox_callback_friend_status(tox, notify_friend_status_cb);
    tox_callback_friend_request(tox, notify_friend_request_cb);
    tox_callback_friend_message(tox, notify_friend_message_cb);

    dht->data_dir = data_location;
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
int dht_bootstrap(DHT *dht, const char *ipv4, const char *ipv6, int port,
                  const uint8_t *address)
{
    Tox *tox = dht->tox;
    bool success;
    TOX_ERR_BOOTSTRAP error;

    assert(dht);
    assert(ipv4 || ipv6);
    assert(port > 0);
    assert(address);

    if (ipv4) {
        success = tox_bootstrap(tox, ipv4, (uint16_t)port, address, &error);
        if (!success) {
            vlogE("DHT: add bootstrap %s:%d error (%d).", ipv4, port, error);
            return DHT_ERROR(ERR_BOOTSTRAP, error);
        }

        success = tox_add_tcp_relay(tox, ipv4, (uint16_t)port, address, &error);
        if (!success)  {
            vlogE("DHT: add tcp relay %s:%d error (%d).", ipv4, port, error);
            return DHT_ERROR(ERR_BOOTSTRAP, error);
        }
    }

    if (ipv6) {
        success = tox_bootstrap(tox, ipv6, (uint16_t)port, address, &error);
        if (!success) {
            vlogE("DHT: add bootstrap %s:%d error (%d).", ipv4, port, error);
            return DHT_ERROR(ERR_BOOTSTRAP, error);
        }

        success = tox_add_tcp_relay(tox, ipv6, (uint16_t)port, address, &error);
        if (!success)  {
            vlogE("DHT: add tcp relay %s:%d error (%d).", ipv4, port, error);
            return DHT_ERROR(ERR_BOOTSTRAP, error);
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
            return DHT_ERROR(ERR_FRIEND_QUERY, error);
        }

        success = tox_friend_get_status_message(tox, friend_list[i], desc,
                                                &error);
        if (!success) {
            vlogE("DHT: get friend status message error (%d).", error);
            return DHT_ERROR(ERR_FRIEND_QUERY, error);
        }

        user_status = tox_friend_get_status(tox, friend_list[i], &error);
        if (error != TOX_ERR_FRIEND_QUERY_OK) {
            vlogE("DHT: get friend user status error (%d).", error);
            return DHT_ERROR(ERR_FRIEND_QUERY, error);
        }

        success = tox_friend_get_public_key(tox, friend_list[i], public_key, &_error);
        if (!success) {
            vlogE("DHT: get friend public key error (%d).", error);
            return DHT_ERROR(ERR_FRIEND_GET_PUBLIC_KEY, error);
        }

        success = cb(friend_list[i], public_key, (int)user_status, desc, desc_len,
                     context);
        if (!success)
            return 0;
    }

    return 0;
}

void dht_store_savedata(DHT *dht)
{
    Tox *tox = dht->tox;
    char *filename;
    FILE *fp;
    uint8_t *data;
    size_t datalen;

    assert(tox);

    filename = (char *)alloca(strlen(dht->data_dir) + strlen(data_filename) + 4);
    sprintf(filename, "%s/%s", dht->data_dir, data_filename);

    fp = fopen(filename, "w");
    if (!fp)
        return;

    datalen = tox_get_savedata_size(tox);
    data = (uint8_t *)alloca(datalen);

    tox_get_savedata(tox, data);

    fwrite(data, sizeof(uint8_t), datalen, fp);
    fclose(fp);
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
        return DHT_ERROR(ERR_SELF_SET_NAME, error);
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
        return DHT_ERROR(ERR_SELF_SET_STATUS_MESSAGE, error);
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
        return DHT_ERROR(ERR_FRIEND_BY_PUBLIC_KEY, error);
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
        return DHT_ERROR(ERR_FRIEND_ADD, error);
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
        return DHT_ERROR(ERR_FRIEND_ADD, error);
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
        vlogW("DHT: send friend message to %lu error (%d).", friend_number,
              error);
        return DHT_ERROR(ERR_FRIEND_SEND_MESSAGE, error);
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
        return DHT_ERROR(ERR_FRIEND_DELETE, error);
    }

    return 0;
}

int dht_get_random_tcp_relay(DHT *dht, char *tcp_relay, size_t buflen,
                             uint8_t *public_key)
{
    Tox *tox = dht->tox;
    int rc;
    uint8_t ip[4];
    struct in_addr in_addr;
    char *addr;

    rc = tox_self_get_random_tcp_relay(tox, ip, public_key);
    if (rc < 0) {
        vlogE("DHT: get random_tcp relay error or no tcp relay connected");
        return -1;
    }

    in_addr.s_addr = *((uint32_t *)ip);
    addr = inet_ntoa(in_addr);

    if (strlen(addr) >= buflen)
        return -1;

    strcpy(tcp_relay, addr);

    return 0;
}


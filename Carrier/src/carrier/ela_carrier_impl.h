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

#ifndef __CARRIER_IMPL_H__
#define __CARRIER_IMPL_H__

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <stdlib.h>
#include <crystal.h>

#include "ela_carrier.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "dht_callbacks.h"
#include "dht.h"

#include "express.h"

#define BOOTSTRAP_DEFAULT_PORT 33445

#define ELA_MAX_EXTENSION_NAME_LEN  (31)

typedef struct DHT {
    uint8_t padding[32];  // reserved for DHT.
} DHT;

typedef struct BootstrapNodeBuf {
    char *ipv4;
    char *ipv6;
    uint16_t port;
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
} BootstrapNodeBuf;

typedef struct ExpressNodeBuf {
    char *ipv4;
    uint16_t port;
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
} ExpressNodeBuf;

typedef struct Preferences {
    char *data_location;
    bool udp_enabled;

    size_t bootstrap_size;
    BootstrapNodeBuf *bootstrap_nodes;

    size_t express_size;
    ExpressNodeBuf *express_nodes;
} Preferences;

typedef struct EventBase EventBase;
struct EventBase {
    void (*handle)(EventBase *, ElaCarrier *);
    list_entry_t le;
};

typedef struct FriendEvent {
    EventBase base;
    ElaFriendInfo fi;
} FriendEvent;

typedef struct OfflineEvent {
    EventBase base;
    char from [ELA_MAX_ADDRESS_LEN + 1];
    int64_t timestamp;
    size_t length;
    uint8_t data[0];
} OfflineEvent;

typedef struct MsgidEvent {
    EventBase base;
    char friendid[ELA_MAX_ADDRESS_LEN + 1];
    int64_t msgid;
    int errcode;
} MsgidEvent;

typedef enum MsgCh {
    MSGCH_DHT = 1,
    MSGCH_EXPRESS = 2,
} MsgCh;

struct ElaCarrier {
    pthread_mutex_t ext_mutex;
    void *session;          //reserved for session extension.
    void *filetransfer;     //reserved for filetransfer extension.
    void *carrier_extesion; //reserved for carrier extension.

    DHT dht;

    Preferences pref;

    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
    uint8_t address[DHT_ADDRESS_SIZE];
    char base58_addr[ELA_MAX_ADDRESS_LEN + 1];

    ElaUserInfo me;
    ElaPresenceStatus presence_status;
    ElaConnectionStatus connection_status;
    bool is_ready;

    ElaCallbacks callbacks;
    ElaGroupCallbacks group_callbacks;
    void *context;

    DHTCallbacks dht_callbacks;

    list_t *friend_events; // for friend_added/removed.
    hashtable_t *friends;

    ExpressConnector *connector;
    uint32_t offmsgid;
    struct timeval express_expiretime;

    hashtable_t *tcallbacks;
    hashtable_t *thistory;

    hashtable_t *tassembly_ireqs;
    hashtable_t *tassembly_irsps;

    hashtable_t *bulkmsgs;

    pthread_mutex_t receipts_mutex;
    hashtable_t *receipts;

    pthread_t main_thread;

    int running;
    int quit;
};

typedef void (*friend_invite_callback)(ElaCarrier *, const char *,
                                       const char *, const void *, size_t, void *);
typedef struct SessionExtension {
    ElaCarrier              *carrier;

    friend_invite_callback  friend_invite_cb;
    void                    *friend_invite_context;

    uint8_t                 reserved[1];
} SessionExtension;

static const char *carrier_extension_name = "carrier";
typedef struct CarrierExtension {
    ElaCarrier              *carrier;

    friend_invite_callback  friend_invite_cb;
    void                    *friend_invite_context;

    void                    *user_callback;
    void                    *user_context;
} CarrierExtension;

CARRIER_API
int ela_leave_all_groups(ElaCarrier *w);

#ifdef __cplusplus
}
#endif

#endif /* __CARRIER_IMPL_H__ */

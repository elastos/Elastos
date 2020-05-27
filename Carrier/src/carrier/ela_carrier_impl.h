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

#ifndef __ELA_CARRIER_IMPL_H__
#define __ELA_CARRIER_IMPL_H__

#include <stdlib.h>

#include <crystal.h>

#include "ela_carrier.h"

#include "dht_callbacks.h"
#include "dht.h"

#include "express.h"

#define DHT_BOOTSTRAP_DEFAULT_PORT 33445

#define MAX_IPV4_ADDRESS_LEN (15)
#define MAX_IPV6_ADDRESS_LEN (47)

#define ELA_MAX_EXTENSION_NAME_LEN  (31)

typedef struct DHT {
    uint8_t padding[32];  // reserved for DHT.
} DHT;

typedef struct DhtBootstrapNodeBuf {
    char ipv4[MAX_IPV4_ADDRESS_LEN + 1];
    char ipv6[MAX_IPV6_ADDRESS_LEN + 1];
    uint16_t port;
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
} DhtBootstrapNodeBuf;

typedef struct ExpressNodeBuf {
    char *ipv4;
    uint16_t port;
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
} ExpressNodeBuf;

typedef struct Preferences {
    char *data_location;
    bool udp_enabled;

    size_t dht_bootstraps_size;
    DhtBootstrapNodeBuf *dht_bootstraps;

    size_t express_nodes_size;
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

typedef enum MsgCh {
    MSGCH_DHT = 1,
    MSGCH_EXPRESS = 2,
} MsgCh;

typedef struct MsgReceiptEvent {
    EventBase base;
    char to[ELA_MAX_ADDRESS_LEN + 1];
    MsgCh msgch;
    int64_t msgid;

    ElaFriendMessageReceiptCallback *callback;
    void *context;

    size_t size;
    uint8_t data[0];
} MsgReceiptEvent;

typedef struct OfflineMsgEvent {
    EventBase base;
    char friendid[ELA_MAX_ADDRESS_LEN + 1];
    union {
        struct {
            int64_t timestamp;
            size_t len;
            uint8_t content[0];
        } msg;
        struct {
            int64_t timestamp;
            size_t len;
            uint8_t gretting[0];
        } req;
        struct {
            int64_t msgid;
            int errcode;
        } stat;
    };
} OfflineMsgEvent;

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
    list_t *friend_msgs;
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
void ela_set_error(int error);

typedef int (*strerror_t)(int errnum, char *, size_t);

CARRIER_API
int ela_register_strerror(int facility, strerror_t strerr);

CARRIER_API
int ela_leave_all_groups(ElaCarrier *w);

#endif /* __ELA_CARRIER_IMPL_H__ */

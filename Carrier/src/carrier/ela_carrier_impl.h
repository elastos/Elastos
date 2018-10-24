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

#include <crypto.h>
#include <linkedhashtable.h>
#include <linkedlist.h>

#include "ela_carrier.h"

#include "dht_callbacks.h"
#include "dht.h"

#define MAX_IPV4_ADDRESS_LEN (15)
#define MAX_IPV6_ADDRESS_LEN (47)

typedef struct DHT {
    uint8_t padding[32];  // reserved for DHT.
} DHT;

typedef struct BootstrapNodeBuf {
    char ipv4[MAX_IPV4_ADDRESS_LEN + 1];
    char ipv6[MAX_IPV6_ADDRESS_LEN + 1];
    uint16_t port;
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
} BootstrapNodeBuf;

typedef struct Preferences {
    char *data_location;
    bool udp_enabled;
    int bootstraps_size;
    BootstrapNodeBuf *bootstraps;
} Preferences;

typedef enum FriendEventType {
    FriendEventType_Added,
    FriendEventType_Removed
} FriendEventType;

typedef struct FriendEvent {
    list_entry_t le;
    FriendEventType type;
    ElaFriendInfo fi;
} FriendEvent;

struct ElaCarrier {
    pthread_mutex_t ext_mutex;
    void *session;  // reserved for session.

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
    void *context;

    DHTCallbacks dht_callbacks;

    list_t *friend_events; // for friend_added/removed.
    hashtable_t *friends;

    hashtable_t *tcallbacks;
    hashtable_t *thistory;

    pthread_t main_thread;

    int running;
    int quit;
};

typedef void (*friend_invite_callback)(ElaCarrier *, const char *,
                                       const char *, size_t, void *);
typedef struct SessionExtension {
    ElaCarrier              *carrier;

    friend_invite_callback  friend_invite_cb;
    void                    *friend_invite_context;

    uint8_t                 reserved[1];
} SessionExtension;

CARRIER_API
void ela_set_error(int error);

typedef int (*strerror_t)(int errnum, char *, size_t);

CARRIER_API
int ela_register_strerror(int facility, strerror_t strerr);

#endif /* __ELA_CARRIER_IMPL_H__ */

#ifndef __ELA_CARRIER_IMPL_H__
#define __ELA_CARRIER_IMPL_H__

#include <stdlib.h>

#include <crypto.h>
#include <linkedhashtable.h>

#include "ela_carrier.h"

#include "dht_callbacks.h"
#include "dht.h"

typedef struct DHT {
    uint8_t padding[32];  // reserved for DHT.
} DHT;

typedef struct BootstrapNode {
    char *ipv4;
    char *ipv6;
    uint16_t port;
    uint8_t public_key[DHT_PUBLIC_KEY_SIZE];
} BootstrapNode;

typedef struct Preferences {
    char *data_location;
    bool udp_enabled;
    int bootstraps_size;
    BootstrapNode *bootstraps;
} Preferences;

struct ElaCarrier {
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

    Hashtable *friends;

    Hashtable *tcallbacks;
    Hashtable *thistory;

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

typedef struct FriendLabelItem {
    uint32_t friend_number;
    char *label;
} FriendLabelItem;

typedef struct TransactedCallback {
    HashEntry he;
    int64_t tid;
    void *callback_func;
    void *callback_context;
} TransactedCallback;

CARRIER_API
void ela_set_error(int error);

#endif /* __ELA_CARRIER_IMPL_H__ */

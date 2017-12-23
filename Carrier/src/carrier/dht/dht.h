#ifndef __DHT_WRAPPER_H__
#define __DHT_WRAPPER_H__

#define DHT_PUBLIC_KEY_SIZE     32U
#define DHT_ADDRESS_SIZE        (32U + sizeof(uint32_t) + sizeof(uint16_t))

typedef struct DHT DHT;

typedef bool (*FriendsIterateCallback)(uint32_t friend_number,
                                       const uint8_t *public_key,
                                       int user_status,
                                       const uint8_t *desc, size_t desc_length,
                                       void *context);

typedef void (*SelfInfoCallback)(const uint8_t *address,
                                 const uint8_t *public_key,
                                 int user_status,
                                 const uint8_t *desc, size_t desc_length,
                                 void *context);

int dht_new(const char *data_location, bool udp_enabled, DHT *dht);

void dht_kill(DHT *dht);

int dht_bootstrap(DHT *dht, const char *ipv4, const char *ipv6, int port,
                  const uint8_t *address);

void dht_self_set_nospam(DHT *dht, uint32_t nospam);

uint32_t dht_self_get_nospam(DHT *dht);

void dht_self_get_secret_key(DHT *dht, uint8_t *secret_key);

void dht_self_set_status(DHT *dht, int status);

int dht_self_get_status(DHT *dht);

int dht_get_self_info(DHT *dht, SelfInfoCallback cb, void *context);

int dht_get_friends(DHT *dht, FriendsIterateCallback cb, void *context);

void dht_store_savedata(DHT *dht);

int dht_iteration_idle(DHT *dht);

void dht_iterate(DHT *dht, void *context);

int dht_self_set_name(DHT *dht, uint8_t *name, size_t length);

int dht_self_set_desc(DHT *dht, uint8_t *status_msg, size_t length);

int dht_get_friend_number(DHT *dht, const uint8_t *public_key,
                          uint32_t *friend_number);


int dht_friend_add(DHT *dht, const uint8_t *address, const uint8_t *msg,
                   size_t length, uint32_t *friend_number);


int dht_friend_add_norequest(DHT *dht, const uint8_t *public_key,
                             uint32_t *friend_number);


int dht_friend_message(DHT *dht, uint32_t friend_number,
                       const uint8_t *data, size_t length);

int dht_friend_delete(DHT *dht, uint32_t friend_number);

int dht_get_random_tcp_relay(DHT *dht, char *tcp_relay, size_t buflen,
                             uint8_t *public_key);

#endif // __DHT_WRAPPER_H__

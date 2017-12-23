#ifndef __DHT_CALLBACKS_H__
#define __DHT_CALLBACKS_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct DHTCallbacks {
    void *context;

    void (*notify_connection)(bool connected, void *context);

    void (*notify_friend_desc)(uint32_t friend_number, const uint8_t *desc,
                               size_t length, void *context);

    void (*notify_friend_connection)(uint32_t friend_number, bool connected,
                                     void *context);

    void (*notify_friend_status)(uint32_t friend_number, int status,
                                 void *context);

    void (*notify_friend_request)(const uint8_t *public_key, const uint8_t *hello,
                                  size_t len, void *context);

    void (*notify_friend_message)(uint32_t friend_number, const uint8_t *message,
                                  size_t length, void *context);
};

typedef struct DHTCallbacks DHTCallbacks;

#ifdef __cplusplus
}
#endif

#endif

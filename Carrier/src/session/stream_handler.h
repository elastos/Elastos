#ifndef __STREAMHANDLER_H__
#define __STREAMHANDLER_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ElaStream ElaStream;
typedef struct StreamHandler StreamHandler;
typedef struct FlexBuffer FlexBuffer;

struct StreamHandler {
    const char *name;

    ElaStream *stream;

    StreamHandler *prev;
    StreamHandler *next;

    int  (*init)            (StreamHandler *handler);
    int  (*prepare)         (StreamHandler *handler);
    int  (*start)           (StreamHandler *handler);
    void (*stop)            (StreamHandler *handler, int error);
    ssize_t (*write)        (StreamHandler *handler, FlexBuffer *buf);
    void (*on_data)         (StreamHandler *handler, FlexBuffer *buf);
    void (*on_state_changed)(StreamHandler *handler, int state);
};

static inline void handler_connect(StreamHandler *handler, StreamHandler *next)
{
    next->next = handler->next;
    next->prev = handler;

    if (handler->next)
        handler->next->prev = next;

    handler->next = next;
}

static inline  StreamHandler *handler_disconnect(StreamHandler *handler)
{
    StreamHandler *next = handler->next;

    if (handler->prev)
        handler->prev->next = handler->next;

    if (handler->next)
        handler->next->prev = handler->prev;

    handler->next = NULL;
    handler->prev = NULL;

    return next;
}

static inline int default_handler_init(StreamHandler *handler)
{
    return handler->next->init(handler->next);
}

static inline int default_handler_prepare(StreamHandler *handler)
{
    return handler->next->prepare(handler->next);
}

static inline int default_handler_start(StreamHandler *handler)
{
    return handler->next->start(handler->next);
}

static inline void default_handler_stop(StreamHandler *handler, int error)
{
    handler->next->stop(handler->next, error);
}

static inline
ssize_t default_handler_write(StreamHandler *handler, FlexBuffer *buf)
{
    return handler->next->write(handler->next, buf);
}

static inline
void default_handler_on_data(StreamHandler *handler, FlexBuffer *buf)
{
    handler->prev->on_data(handler->prev, buf);
}

static inline
void default_handler_on_state_changed(StreamHandler *handler, int state)
{
    handler->prev->on_state_changed(handler->prev, state);
}

int crypto_handler_create(ElaStream *s, StreamHandler **handler);

int reliable_handler_create(ElaStream *s, StreamHandler **handler);

#ifdef __cplusplus
}
#endif

#endif /* __STREAMHANDLER_H__ */

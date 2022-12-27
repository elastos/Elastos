#ifndef __CARRIER_EXTENSION_H__
#define __CARRIER_EXTENSION_H__

#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

#include <ela_carrier.h>

typedef void ExtensionInviteCallback(ElaCarrier *carrier, const char *from,
                                     const void *data, size_t len, void *context);
extern int extension_init(ElaCarrier *carrier, ExtensionInviteCallback *callback, void *context);
extern void extension_cleanup(ElaCarrier *carrier);
typedef void ExtensionInviteReplyCallback(ElaCarrier *carrier, const char *from,
                                          int status, const char *reason,
                                          const void *data, size_t len, void *context);
extern int extension_invite_friend(ElaCarrier *carrier, const char *to,
                                   const void *data, size_t len,
                                   ExtensionInviteReplyCallback *callback,
                                   void *context);
extern int extension_reply_friend_invite(ElaCarrier *carrier, const char *to,
                                         int status, const char *reason,
                                         const void *data, size_t len);

#endif /* __CARRIER_EXTENSION_H__ */

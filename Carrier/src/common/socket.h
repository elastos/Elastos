#ifndef __SOCKET_H__
#define __SOCKET_H__

#if !defined(_WIN32) && !defined(_WIN64)
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif /* WIN32 */

#include <common_export.h>

#ifdef __cplusplus
extern "C" {
#endif

/* for Winsock compatible */
#if !defined(_WIN32) && !defined(_WIN64)
#define SOCKET              int
#define INVALID_SOCKET      -1
#endif

#define SOCKET_ADDR_MAX_LEN    64

COMMON_API
SOCKET socket_create(int type, const char *host, const char *port);

COMMON_API
SOCKET socket_connect(const char *host, const char *port);

COMMON_API
int socket_close(SOCKET s);

COMMON_API
int socket_set_nonblock(SOCKET s);

COMMON_API
const char *socket_addr_name(const struct sockaddr *addr, char *dest, size_t size);

COMMON_API
int socket_addr_from_name(const char *host, const char *port, int type,
                          struct sockaddr *addr, socklen_t *socklen);

COMMON_API
const char *socket_local_name(SOCKET sock, char *dest, size_t size);

COMMON_API
const char *socket_local_addr(SOCKET sock, char *dest, size_t size);

COMMON_API
const char *socket_local_port(SOCKET sock, char *dest, size_t size);

COMMON_API
const char *socket_remote_name(SOCKET sock, char *dest, size_t size);

COMMON_API
int socket_local_name_equal(SOCKET sock, int type,
                            const char *host, const char *port);

COMMON_API
const char *get_default_address(char *addr, size_t size);

COMMON_API
int get_all_addresses(char **addrs[]);

COMMON_API
void free_addresses(char *addrs[]);

COMMON_API
int socket_errno(void);

#ifdef __cplusplus
}
#endif

#endif /* __SOCKET_H__ */

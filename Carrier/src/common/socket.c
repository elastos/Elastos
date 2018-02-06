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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>

#endif

#include "socket.h"

SOCKET socket_create(int type, const char *host, const char *port)
{
    SOCKET sock = INVALID_SOCKET;
    struct addrinfo hints;
    struct addrinfo *ai;
    struct addrinfo *p;
    int rc;

    if (!host && !port) {
        sock = socket(AF_INET, type, IPPROTO_TCP);
        return sock;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = type;
    if (type == SOCK_STREAM)
        hints.ai_protocol = IPPROTO_TCP;
    else if (type == SOCK_DGRAM)
        hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo(host, port, &hints, &ai);
    if (rc != 0)
        return INVALID_SOCKET;

    for (p = ai; p; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == INVALID_SOCKET)
            continue;

        int set = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&set, sizeof(set));
        if (bind(sock, p->ai_addr, p->ai_addrlen) != 0) {
            socket_close(sock);
            sock = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(ai);
    return sock;
}

SOCKET socket_connect(const char *host, const char *port)
{
    SOCKET sock = INVALID_SOCKET;
    struct addrinfo hints;
    struct addrinfo *ai;
    struct addrinfo *p;
    int rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    rc = getaddrinfo(host, port, &hints, &ai);
    if (rc != 0)
        return INVALID_SOCKET;

    for (p = ai; p; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == INVALID_SOCKET) {
            continue;
        }

        if (p->ai_socktype == SOCK_STREAM) {
            if (connect(sock, p->ai_addr, p->ai_addrlen) != 0) {
                socket_close(sock);
                sock = INVALID_SOCKET;
                continue;
            }
        }

        break;
    }

    freeaddrinfo(ai);
    return sock;
}

int socket_set_nonblock(SOCKET s)
{
#if 0
#if defined(_WIN32) || defined (_WIN64)
    u_long mode = 1;
    return ioctlsocket(s, FIONBIO, &mode);
#else
    return fcntl(s, F_SETFL, O_NONBLOCK, 1);
#endif
#else
    return 0;
#endif
}

int socket_close(SOCKET s)
{
#if !defined(_WIN32) && !defined(_WIN64)
    return close(s);
#else
    return closesocket(s);
#endif
}

const char *socket_addr_name(const struct sockaddr *addr, char *dest, size_t size)
{
    int port;
    size_t len;
    const char *tmp;
    char buf[SOCKET_ADDR_MAX_LEN];

    if (!addr || !dest || size <= 0) {
        errno = EINVAL;
        return NULL;
    }

    tmp = inet_ntop(addr->sa_family, &((struct sockaddr_in *)addr)->sin_addr,
              buf, sizeof(buf));
    if (!tmp)
        return NULL;

    len = strlen(buf);
    if (len >= size) {
        errno = ENOSPC;
        return NULL;
    }
    
    port = ntohs(((struct sockaddr_in *)addr)->sin_port);
    sprintf(buf + len, ":%d", port);

    if (strlen(buf) >= size) {
        errno = ENOSPC;
        return NULL;
    }

    strcpy(dest, buf);
    return dest;
}

const char *socket_local_name(SOCKET sock, char *dest, size_t size)
{
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);

    if (sock < 0 || !dest || size <= 0) {
        errno = EINVAL;
        return NULL;
    }

    if (getsockname(sock, (struct sockaddr *)&addr, &addrlen) < 0)
        return NULL;
        
    return socket_addr_name((const struct sockaddr *)&addr, dest, size);
}

const char *socket_local_addr(SOCKET sock, char *dest, size_t size)
{
    struct sockaddr_storage addr;
    struct sockaddr_in *inaddr;
    socklen_t addrlen = sizeof(addr);

    if (sock < 0 || !dest || size <= 0) {
        errno = EINVAL;
        return NULL;
    }

    if (getsockname(sock, (struct sockaddr *)&addr, &addrlen) < 0)
        return NULL;

    inaddr = (struct sockaddr_in *)&addr;
    return inet_ntop(inaddr->sin_family, &inaddr->sin_addr, dest, (socklen_t)size);
}

const char *socket_local_port(SOCKET sock, char *dest, size_t size)
{
    struct sockaddr_storage addr;
    struct sockaddr_in *inaddr;
    int port;
    socklen_t addrlen = sizeof(addr);

    if (sock < 0 || !dest || size <= 0) {
        errno = EINVAL;
        return NULL;
    }

    if (getsockname(sock, (struct sockaddr *)&addr, &addrlen) < 0)
        return NULL;

    inaddr = (struct sockaddr_in *)&addr;
    port = ntohs(inaddr->sin_port);
    snprintf(dest, size, "%d", port);
    return dest;
}

const char *socket_remote_name(SOCKET sock, char *dest, size_t size)
{
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);

    if (sock < 0 || !dest || size <= 0) {
        errno = EINVAL;
        return NULL;
    }

    if (getpeername(sock, (struct sockaddr *)&addr, &addrlen) < 0)
        return NULL;

    return socket_addr_name((const struct sockaddr *)&addr, dest, size);
}

int socket_local_name_equal(SOCKET sock, int type,
                            const char *host, const char *port)
{
    int equal = 0;

    struct addrinfo hints;
    struct addrinfo *ai;
    struct addrinfo *p;

    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);

    getsockname(sock, (struct sockaddr *)&addr, &addrlen);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = addr.ss_family;
    hints.ai_socktype = type;
    if (type == SOCK_STREAM)
        hints.ai_protocol = IPPROTO_TCP;
    else if (type == SOCK_DGRAM)
        hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(host, port, &hints, &ai) != 0)
        return 0;

    for (p = ai; p; p = p->ai_next) {
        if (addrlen == p->ai_addrlen
            && memcmp(&addr, p->ai_addr, addrlen) == 0) {
            equal = 1;
            break;
        }
    }

    freeaddrinfo(ai);
    return equal;
}

int socket_addr_from_name(const char *host, const char *port, int type,
                          struct sockaddr *addr, socklen_t *socklen)
{
    struct addrinfo hints;
    struct addrinfo *ai;
    struct addrinfo *p;
    int rc;

    if (!socklen)
        return EINVAL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = type;
    if (type == SOCK_STREAM)
        hints.ai_protocol = IPPROTO_TCP;
    else if (type == SOCK_DGRAM)
        hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo(host, port, &hints, &ai);
    if (rc < 0)
        return EINVAL;

    for (p = ai; p; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            if (p->ai_addrlen > *socklen)
                continue;

            memcpy(addr, p->ai_addr, p->ai_addrlen);
            *socklen = p->ai_addrlen;
            freeaddrinfo(ai);
            return 0;
        }
    }

    freeaddrinfo(ai);
    return EINVAL;
}

#define MAX_IP_ADDRESSES        16

struct ip_addresses {
    char *addrs[MAX_IP_ADDRESSES];
    char buffer[MAX_IP_ADDRESSES * 16];
};

int get_all_addresses(char **addrs[])
{
    struct ifaddrs *ads;
    struct ifaddrs *p;
    struct ip_addresses *result;
    char *cur;
    socklen_t len;
    int cnt;

    result = (struct ip_addresses *)calloc(1, sizeof(struct ip_addresses));
    if (result == NULL)
        return -1;

    if (getifaddrs(&ads) < 0) {
        free(result);
        return -1;
    }

    cnt = 0;
    cur = result->buffer;
    len = (socklen_t)sizeof(result->buffer);

    for (p = ads; cnt < 16 && p != NULL; p = p->ifa_next) {
        if (p->ifa_addr && p->ifa_addr->sa_family == AF_INET
            && (p->ifa_flags & IFF_UP) == IFF_UP && (p->ifa_flags & IFF_LOOPBACK) == 0) {
            struct sockaddr_in *inaddr = (struct sockaddr_in *)p->ifa_addr;
            const char *ret = inet_ntop(inaddr->sin_family, &inaddr->sin_addr,
                        cur, len);
            if (ret) {
                size_t l = strlen(ret) + 1;
                result->addrs[cnt++] = cur;

                len -= l;
                cur += l;
            }
        }
    }

    freeifaddrs(ads);

    if (cnt)
        *addrs = result->addrs;
    else {
        *addrs = NULL;
        free(result);
    }

    return cnt;
}

void free_addresses(char *addrs[])
{
    if (addrs)
        free(addrs);
}

const char *get_default_address(char *addr, size_t size)
{
    struct addrinfo hints;
    struct addrinfo *ai;
    const char *ret;
    SOCKET sock;
    int rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo("1.1.1.1", "53", &hints, &ai);
    if (rc != 0)
        return NULL;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        freeaddrinfo(ai);
        return NULL;
    }

    rc = connect(sock, ai->ai_addr, ai->ai_addrlen);
    if (rc < 0) {
        freeaddrinfo(ai);
        socket_close(sock);
        return NULL;
    }

    ret = socket_local_addr(sock, addr, (socklen_t)size);

    freeaddrinfo(ai);
    socket_close(sock);
    return ret;
}

int socket_errno(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return WSAGetLastError();
#else
    return errno;
#endif
}

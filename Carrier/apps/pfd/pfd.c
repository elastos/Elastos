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
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <crystal.h>
#include <assert.h>
#include <pthread.h>
#include <fcntl.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <ela_carrier.h>
#include <ela_session.h>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#endif

#include "carrier_config.h"
#include "config.h"

#define CONFIG_NAME   "elapfd.conf"

static const char *default_config_files[] = {
    "./"CONFIG_NAME,
    "../etc/carrier/"CONFIG_NAME,
#if !defined(_WIN32) && !defined(_WIN64)
    "/usr/local/etc/carrier/"CONFIG_NAME,
    "/etc/carrier/"CONFIG_NAME,
#endif
    NULL
};

static PFConfig config;

static ElaCarrier *carrier;

typedef struct SessionEntry {
    hash_entry_t he;
    ElaSession *session;
} SessionEntry;

hashtable_t *sessions;

// Client only
static ElaSession *cli_session;
static int cli_streamid;

static void session_entry_destroy(void *p)
{
    SessionEntry *entry = (SessionEntry *)p;
    if (entry && entry->session) {
        char peer[ELA_MAX_ID_LEN*2+8];

        ela_session_get_peer(entry->session, peer, sizeof(peer));
        ela_session_close(entry->session);
        vlogI("Session to %s closed", peer);
    }
}

static void add_session(ElaSession *ws)
{
    assert(ws);

    SessionEntry *entry = rc_alloc(sizeof(SessionEntry), session_entry_destroy);
    if (!entry) {
        perror("Out of memory");
        exit(-1);
    }

    entry->he.data = entry;
    entry->he.key = ws;
    entry->he.keylen = sizeof(ElaSession *);
    entry->session = ws;

    hashtable_put(sessions, &entry->he);

    deref(entry);
}

static int exist_session(ElaSession *ws)
{
    if (sessions)
        return hashtable_exist(sessions, ws, sizeof(ElaSession *));
    else
        return 0;
}

static void delete_session(ElaSession *ws)
{
    if (!sessions)
        return;

    SessionEntry *entry = hashtable_remove(sessions, ws, sizeof(ElaSession *));
    if (entry) {
        if (config.mode == MODE_CLIENT) {
            cli_session = NULL;
            cli_streamid = -1;
        }

        deref(entry);
    }
}

static void setup_portforwardings(void);

// Client only
static void peer_connection_changed(ElaConnectionStatus status)
{
    if (status == ElaConnectionStatus_Connected) {
        vlogI("Portforwarding server is online, setup portforwardings...");
        setup_portforwardings();
    } else {
        vlogI("Portforwarding server is being offline.");

        // Close current session if exist
        if (cli_session)
            delete_session(cli_session);

        vlogI("Portforwarding service will available when server peer online.");
    }
}

// Client only
static void carrier_ready(ElaCarrier *w, void *context)
{
    int rc;
    char uid[ELA_MAX_ID_LEN+1];
    char addr[ELA_MAX_ADDRESS_LEN+1];

    vlogI("Carrier is ready!");
    vlogI("User ID: %s", ela_get_userid(w, uid, sizeof(uid)));
    vlogI("Address: %s", ela_get_address(w, addr, sizeof(addr)));

    if (config.mode == MODE_SERVER)
        return; // Server mode: do nothing.

    const char *friendid = config.serverid;

    if (!ela_is_friend(w, friendid)) {
        vlogI("Portforwarding server not friend yet, send friend request...");

        rc = ela_add_friend(w, config.server_address, "Elastos Carrier PFD/C");
        if (rc < 0) {
            vlogE("Add portforwarding server as friend failed (0x%8X)",
                  ela_get_error());
        } else {
            vlogI("Add portforwarding server as friend success!");
        }
    } else {
        ElaFriendInfo fi;
        ela_get_friend_info(w, friendid, &fi);
        peer_connection_changed(fi.status);
    }
}

// Client only
static void friend_connection(ElaCarrier *w, const char *friendid,
                              ElaConnectionStatus status, void *context)
{
    if (config.mode == MODE_SERVER)
        return; // Server mode: do nothing.

    if (strcmp(friendid, config.serverid) != 0)
        return; // Ignore uninterested peer

    peer_connection_changed(status);
}

// Server and client
static void friend_request(ElaCarrier *w, const char *userid,
            const ElaUserInfo *info, const char *hello, void *context)
{
    int rc;
    int status = -1;

    if (config.mode == MODE_SERVER &&
            hashtable_exist(config.users, userid, strlen(userid))) {
        status = 0;
    }

    vlogI("%s friend request from %s.", status == 0 ? "Accept" : "Refuse",
            info->userid);

    if (status != 0) {
        vlogI("Skipped unathorized friend request from %s.", userid);
        return;
    } else {
        rc = ela_accept_friend(w, userid);
        if (rc < 0) {
            vlogE("Accept friend request failed(%08X).", ela_get_error());
            return;
        } else {
            vlogI("Accepted user %s to be friend.", userid);
        }
    }
}

// Client only
static void session_request_complete(ElaSession *ws, const char *bundle, int status,
                const char *reason, const char *sdp, size_t len, void *context)
{
    const char *state_name[] = {
        "raw",
        "initialized",
        "transport ready",
        "connecting",
        "connected",
        "deactived",
        "closed",
        "error"
    };
    ElaStreamState state;
    int rc;

    if (status != 0) {
        vlogE("Session request complete with error(%d:%s).", status, reason);
        return;
    }

    rc = ela_stream_get_state(ws, cli_streamid, &state);
    while (rc == 0 && state < ElaStreamState_transport_ready) {
        usleep(100);
        rc = ela_stream_get_state(ws, cli_streamid, &state);
    }

    if (rc < 0) {
        vlogE("Acquire stream state in session failed(%08X).", ela_get_error());
        delete_session(ws);
        return;
    }

    if (state != ElaStreamState_transport_ready) {
        vlogE("Session stream state wrong %s.", state_name[state]);
        delete_session(ws);
        return;
    }

    rc = ela_session_start(ws, sdp, len);
    if (rc < 0) {
        vlogE("Start session to portforwarding server peer failed(%08X).", ela_get_error());
        delete_session(ws);
    } else
        vlogI("Start session to portforwarding server peer success.");
}

// Server and client
static void stream_state_changed(ElaSession *ws, int stream,
                                 ElaStreamState state, void *context)
{
    int rc;
    char peer[ELA_MAX_ID_LEN*2+8];

    ela_session_get_peer(ws, peer, sizeof(peer));

    if (state == ElaStreamState_failed
            || state == ElaStreamState_closed) {
        vlogI("Session to %s closed %s.", peer,
              state == ElaStreamState_closed ? "normally" : "on connection error");

        if (config.mode == MODE_SERVER && exist_session(ws))
            free(ela_session_get_userdata(ws));

        delete_session(ws);
        return;
    }

    if (config.mode == MODE_CLIENT) {
        if (state == ElaStreamState_initialized) {
            rc = ela_session_request(ws, NULL, session_request_complete, NULL);
            if (rc < 0) {
                vlogE("Session request to portforwarding server peer failed(%08X)", ela_get_error());
                delete_session(ws);
            } else {
                vlogI("Session request to portforwarding server success.");
            }
        } else if (state == ElaStreamState_connected) {
            hashtable_iterator_t it;

            hashtable_iterate(config.services, &it);
            while (hashtable_iterator_has_next(&it)) {
                PFService *svc;
                hashtable_iterator_next(&it, NULL, NULL, (void **)&svc);

                int rc = ela_stream_open_port_forwarding(ws, stream,
                            svc->name, PortForwardingProtocol_TCP, svc->host, svc->port);
                if (rc <= 0)
                    vlogE("Open portforwarding for service %s on %s:%s failed(%08X).",
                          svc->name, svc->host, svc->port, ela_get_error());
                else
                    vlogI("Open portforwarding for service %s on %s:%s success.",
                          svc->name, svc->host, svc->port);

                deref(svc);
            }
        }
    } else {
        if (state == ElaStreamState_initialized) {
            rc = ela_session_reply_request(ws, NULL, 0, NULL);
            if (rc < 0) {
                vlogE("Session request from %s, reply failed(%08X)", peer, ela_get_error());
                free(ela_session_get_userdata(ws));
                delete_session(ws);
                return;
            }
            vlogI("Session request from %s, accepted!", peer);
        } else if (state == ElaStreamState_transport_ready) {
            char *sdp = (char *)ela_session_get_userdata(ws);

            rc = ela_session_start(ws, sdp, strlen(sdp));
            ela_session_set_userdata(ws, NULL);
            free(sdp);
            if (rc < 0) {
                vlogE("Start session to %s failed(%08X).", peer, ela_get_error());
                delete_session(ws);
            } else
                vlogI("Start session to %s success.", peer);
        }
    }
}

// Server and client
static void session_request_callback(ElaCarrier *w, const char *from, const char *bundle,
                                   const char *sdp, size_t len, void *context)
{
    ElaSession *ws;
    PFUser *user;
    char userid[ELA_MAX_ID_LEN + 1];
    char *p;
    int i;
    int rc;
    int options = config.options;

    ElaStreamCallbacks stream_callbacks;

    vlogI("Session request from %s", from);

    ws = ela_session_new(w, from);
    if (ws == NULL) {
        vlogE("Create session failed(%08X).", ela_get_error());
        return;
    }

    if (config.mode == MODE_CLIENT) {
        // Client mode: just refuse the request.
        vlogI("Refuse session request from %s.", from);
        ela_session_reply_request(ws, NULL, -1, "Refuse");
        ela_session_close(ws);
        return;
    }

    // Server prepare the portforwarding services

    p = strchr(from, '@');
    if (p) {
        size_t len = p - from;
        strncpy(userid, from, len);
        userid[len] = 0;
    } else
        strcpy(userid, from);

    user = (PFUser *)hashtable_get(config.users, userid, strlen(userid));
    if (user == NULL) {
        // Not in allowed user list. Refuse session request.
        vlogI("Refuse session request from %s.", from);
        ela_session_reply_request(ws, NULL, -1, "Refuse");
        ela_session_close(ws);
        return;
    }

    for (i = 0; user->services[i] != NULL; i++) {
        PFService *svc = (PFService *)hashtable_get(config.services,
                            user->services[i], strlen(user->services[i]));

        rc = ela_session_add_service(ws, svc->name,
                        PortForwardingProtocol_TCP, svc->host, svc->port);
        if (rc < 0)
            vlogE("Prepare service %s for %s failed(%08X).",
                  svc->name, userid, ela_get_error());
        else
            vlogI("Add service %s for %s.", svc->name, userid);
    }

    p = strdup(sdp);
    ela_session_set_userdata(ws, p);

    add_session(ws);
    memset(&stream_callbacks, 0, sizeof(stream_callbacks));
    stream_callbacks.state_changed = stream_state_changed;
    rc = ela_session_add_stream(ws, ElaStreamType_application,
                    options | ELA_STREAM_MULTIPLEXING | ELA_STREAM_PORT_FORWARDING,
                    &stream_callbacks, NULL);
    if (rc <= 0) {
        vlogE("Session request from %s, can not add stream(%08X)", from, ela_get_error());
        ela_session_reply_request(ws, NULL, -1, "Error");
        delete_session(ws);
        free(p);
    }
}

// Client only
static void setup_portforwardings(void)
{
    ElaStreamCallbacks stream_callbacks;
    int options = config.options;

    // May be previous session not closed properly.
    if (cli_session != NULL)
        delete_session(cli_session);

    cli_session = ela_session_new(carrier, config.serverid);
    if (cli_session == NULL) {
        vlogE("Create session to portforwarding server failed(%08X).", ela_get_error());
        return;
    }

    vlogI("Created session to portforwarding server.");

    add_session(cli_session);

    memset(&stream_callbacks, 0, sizeof(stream_callbacks));
    stream_callbacks.state_changed = stream_state_changed;

    cli_streamid = ela_session_add_stream(cli_session, ElaStreamType_application,
                options | ELA_STREAM_MULTIPLEXING | ELA_STREAM_PORT_FORWARDING,
                &stream_callbacks, NULL);
    if (cli_streamid <= 0) {
        vlogE("Add stream to session failed(%08X)", ela_get_error());
        delete_session(cli_session);
    } else {
        vlogI("Add stream %d to session success.", cli_streamid);
    }
}

static void stop(void)
{
    hashtable_t *ss = sessions;

    sessions = NULL;
    if (ss)
        deref(ss);

    if (carrier) {
        ela_session_cleanup(carrier);
        ela_kill(carrier);
        carrier = NULL;
    }

    free_config(&config);
}

static void signal_handler(int signum)
{
    stop();
}

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>

int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4311)
#endif

static uint32_t session_hash_code(const void *key, size_t len)
{
    return (uint32_t)key;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

static int session_hash_compare(const void *key1, size_t len1,
                                const void *key2, size_t len2)
{
    if (key1 == key2)
        return 0;
    else if (key1 < key2)
        return -1;
    else
        return 1;
}

const char *get_config_path(const char *config_file, const char *config_files[])
{
    const char **file = config_file ? &config_file : config_files;

    for (; *file; ) {
        int fd = open(*file, O_RDONLY);
        if (fd < 0) {
            if (*file == config_file)
                file = config_files;
            else
                file++;

            continue;
        }

        close(fd);

        return *file;
    }

    return NULL;
}

static void usage(void)
{
    printf("Carrier portforwarding utility.\n");
    printf("Usage: elapfd [OPTION]...\n");
    printf("\n");
    printf("First run options:\n");
    printf("  -c, --config=CONFIG_FILE  Set config file path.\n");
    printf("      --udp-enabled=0|1     Enable UDP, override the option in config.\n");
    printf("      --log-level=LEVEL     Log level(0-7), override the option in config.\n");
    printf("      --log-file=FILE       Log file name, override the option in config.\n");
    printf("      --data-dir=PATH       Data location, override the option in config.\n");
    printf("\n");
    printf("Debugging options:\n");
    printf("      --debug               Wait for debugger attach after start.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    ElaCallbacks callbacks;
    const char *config_file = NULL;
    int wait_for_attach = 0;
    int rc;
    int opt;
    int idx;

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
#if !defined(_WIN32) && !defined(_WIN64)
    // Uncatchable: signal(SIGKILL, signal_handler);
    signal(SIGHUP, signal_handler);
#endif

    struct option options[] = {
        { "config",         required_argument,  NULL, 'c' },
        { "udp-enabled",    required_argument,  NULL, 1 },
        { "log-level",      required_argument,  NULL, 2 },
        { "log-file",       required_argument,  NULL, 3 },
        { "data-dir",       required_argument,  NULL, 4 },
        { "debug",          no_argument,        NULL, 5 },
        { "help",           no_argument,        NULL, 'h' },
        { NULL,             0,                  NULL, 0 }
    };

    while ((opt = getopt_long(argc, argv, "c:h?", options, &idx)) != -1) {
        switch (opt) {
        case 'c':
            config_file = optarg;
            break;

        case 1:
        case 2:
        case 3:
        case 4:
            break;

        case 5:
            wait_for_attach = 1;
            break;

        case 'h':
        case '?':
        default:
            usage();
            exit(-1);
        }
    }

    if (wait_for_attach) {
        printf("Wait for debugger attaching, process id is: %d.\n", getpid());
        printf("After debugger attached, press any key to continue......");
        getchar();
    }

    config_file = get_config_path(config_file, default_config_files);
    if (!config_file) {
        printf("Error: Missing config file.\n");
        printf("\nUSAGE: elapfd [-c CONFIG_FILE]\n\n");
        return -1;
    }

    if (!load_config(config_file, &config)) {
        return -1;
    }

    carrier_config_update(&config.ela_options, argc, argv);

    sessions = hashtable_create(16, 1, session_hash_code, session_hash_compare);
    if (!sessions) {
        free_config(&config);
        return -1;
    }

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.ready = carrier_ready;
    callbacks.friend_connection = friend_connection;
    callbacks.friend_request = friend_request;

    carrier = ela_new(&config.ela_options, &callbacks, &config);
    if (!carrier) {
        fprintf(stderr, "Can not create Carrier instance (%08X).\n",
                ela_get_error());
        stop();
        return -1;
    }

    rc = ela_session_init(carrier);
    if (rc < 0) {
        fprintf(stderr, "Can not initialize Carrier session extension (%08X).",
                ela_get_error());
        stop();
        return -1;
    }

    rc = ela_session_set_callback(carrier, NULL, session_request_callback, NULL);
    if (rc < 0) {
        fprintf(stderr, "Can not set callbacks (%08X).",
                ela_get_error());
        stop();
        return -1;
    }

    rc = ela_run(carrier, 500);
    if (rc < 0)
        fprintf(stderr, "Can not start Carrier.\n");

    stop();
    return rc;
}

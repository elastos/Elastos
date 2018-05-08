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
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <android/log.h>

#ifdef __linux__
#define __USE_GNU
#include <pthread.h>
#include <assert.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#else
#include <pthread.h>
#endif

#define TEST_SPEED_MSG      "test_speed"
#define TEST_SPEED_ACK_MSG  "test_speed_ack"
#define ACCEPT_TEST_SPEED   "accept"
#define REFUSE_TEST_SPEED   "refuse"
#define DATA_LEN_MSG        "data_len"
#define MD5_CHECKSUM_MSG    "md5_checksum"

#include "ela_carrier.h"
#include "ela_session.h"
#include "java_opt.h"
#include "md5.h"
#include "speedtest.h"

typedef enum RUNNING_MODE {
    INITIAL_MODE,
    ACTIVE_MODE,
    PASSIVE_MODE
} RUNNING_MODE;

static RUNNING_MODE g_mode = INITIAL_MODE;
static char g_peer_id[ELA_MAX_ID_LEN+1] = {0};
static char g_md5[33] = {0};
static char g_transferred_file[1024] = {0};
static char g_received_file[1024] = {0};
static int g_bytes_transferred = 0;
static int g_data_len = 0;
static int g_stream_id = 0;

ElaCarrier *g_carrier = NULL;
JavaVM *g_jvm = NULL;
JNIEnv *g_env = NULL;
jobject g_main_activity = NULL;
jobject g_asset_manager = NULL;

pthread_mutex_t g_screen_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;

static struct {
    ElaSession *ws;
    int unchanged_streams;
    char remote_sdp[2048];
    size_t sdp_len;
    int bulk_mode;
    size_t packets;
    size_t bytes;
    struct timeval first_stamp;
    struct timeval last_stamp;
} session_ctx;

static void session_init(ElaCarrier *);
static void session_new(ElaCarrier *, int, char **);
static void session_request(ElaCarrier *);
static void session_reply_request(ElaCarrier *, int, char **);
static void stream_add(ElaCarrier *, int, char **);
static void stream_bulk_write(ElaCarrier *, int, char **);
static void stream_bulk_receive(ElaCarrier *, int, char **);
static void stream_get_info(ElaCarrier *, int, char **);

static void log_print(const char *format, va_list args)
{
    pthread_mutex_lock(&g_screen_lock);
    vfprintf(stdout, format, args);
    pthread_mutex_unlock(&g_screen_lock);
}

static void output(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    pthread_mutex_lock(&g_screen_lock);
#ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_INFO, "Carrier", format, args);
#else
    vfprintf(stdout, format, args);
#endif
    pthread_mutex_unlock(&g_screen_lock);

    va_end(args);
}

const char *presence_name[] = {
    "none",    // None;
    "away",    // Away;
    "busy",    // Busy;
};

char *get_userid(char *userid, size_t len)
{
    char *p = NULL;

    if ((userid == NULL) || (len < ELA_MAX_ID_LEN + 1))
        return NULL;

    p = ela_get_userid(g_carrier, userid, len);

    return p;
}

char *get_address(char *address, size_t len)
{
    char *p = NULL;

    if ((address == NULL) || (len < ELA_MAX_ID_LEN + 1))
        return NULL;

    p = ela_get_address(g_carrier, address, len);

    return p;
}

static void friend_add(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 1) {
        output("Invalid invocation.\n");
        return;
    }

    rc = ela_add_friend(w, argv[0], "Hello");
    if (rc == 0) {
        g_mode = ACTIVE_MODE;
        output("Request to add a new friend successfully.\n");
    }
    else
        output("Request to add a new friend unsuccessfully(0x%x).\n",
                ela_get_error());
}

void add_friend(const char *address)
{
    int rc;
    char *addr_arg[1] = {(char*)address};
    char *p = NULL;

    if (address == NULL) {
        output("Invalid invocation.\n");
        return;
    }

    p = ela_get_id_by_address(address, g_peer_id, ELA_MAX_ID_LEN + 1);
    if (p != NULL) {
        char *arg[1] = {g_peer_id};
        friend_add(g_carrier, 1, addr_arg);
    } else {
        output("Got user ID unsuccessfully.\n");
    }
}

int accept_friend(const char *userid)
{
    int rc;

    if (userid == NULL) {
        output("Invalid invocation.\n");
        return -1;
    }

    rc = ela_accept_friend(g_carrier, userid);
    if (rc == 0)
        output("Accept friend request successfully.\n");
    else
        output("Accept friend request unsuccessfully(0x%x).\n", ela_get_error());

    return rc;
}

int remove_friend(const char *userid)
{
    int rc;

    if (userid == NULL)
        return -1;

    rc = ela_remove_friend(g_carrier, userid);
    if (rc == 0)
        output("Remove friend %s success.\n", userid);
    else
        output("Remove friend %s failed (0x%x).\n", userid, ela_get_error());

    return rc;
}

static bool get_friends_callback(const ElaFriendInfo *info, void *context)
{
    ElaFriendInfoNode *head = NULL;
    ElaFriendInfoNode *node = NULL;

    assert(context);

    head = (ElaFriendInfoNode*)context;

    if (info == NULL)
        return false;

    node = malloc(sizeof(ElaFriendInfoNode));
    if (node == NULL)
        return false;

    strcpy(&node->info, info);
    node->next = NULL;

    if (head == NULL) {
        head->next = node;
    } else {
        node->next = head->next;
        head->next = node;
    }

    return true;
}

ElaFriendInfoNode *get_friends(void **context)
{
    int ret = 0;

    if (context == NULL)
        return NULL;

    ret = ela_get_friends(g_carrier, get_friends_callback, *context);

    return *context;
}

int free_friends(void *context)
{
    ElaFriendInfoNode *head = NULL;
    ElaFriendInfoNode *p = NULL;

    if (context == NULL)
        return -1;

    head = (ElaFriendInfoNode*)context;
    p = head->next;
    while (p) {
        head->next = p->next;
        free(p);
        p = head->next;
    }

    return 0;
}

static int first_friends_item = 1;

static const char *connection_name[] = {
    "online",
    "offline"
};

/* This callback share by list_friends and global
 * friend list callback */
static bool friends_list_callback(ElaCarrier *w, const ElaFriendInfo *friend_info,
                                 void *context)
{
    static int count;

    if (first_friends_item) {
        count = 0;
        output("Friend list from carrier network:\n");
        output("  %-46s %8s %s\n", "ID", "Connection", "Label");
        output("  %-46s %8s %s\n", "----------------", "----------", "-----");
    }

    if (friend_info) {
        output("  %-46s %8s %s\n", friend_info->user_info.userid,
               connection_name[friend_info->status], friend_info->label);
        first_friends_item = 0;
        count++;
    } else {
        /* The list ended */
        output("  ----------------\n");
        output("Total %d friends.\n", count);

        first_friends_item = 1;
    }

    return true;
}

/* This callback share by list_friends and global
 * friend list callback */
static void display_friend_info(const ElaFriendInfo *fi)
{
    output("           ID: %s\n", fi->user_info.userid);
    output("         Name: %s\n", fi->user_info.name);
    output("  Description: %s\n", fi->user_info.description);
    output("       Gender: %s\n", fi->user_info.gender);
    output("        Phone: %s\n", fi->user_info.phone);
    output("        Email: %s\n", fi->user_info.email);
    output("       Region: %s\n", fi->user_info.region);
    output("        Label: %s\n", fi->label);
    output("     Presence: %s\n", presence_name[fi->presence]);
    output("   Connection: %s\n", connection_name[fi->status]);
}

static void friend_added_callback(ElaCarrier *w, const ElaFriendInfo *info,
                                  void *context)
{
    output("New friend added:\n");
    display_friend_info(info);
}

static void friend_removed_callback(ElaCarrier *w, const char *friendid,
                                    void *context)
{
    output("Friend %s removed!\n", friendid);
}

static void send_message(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 2) {
        output("Invalid invocation.\n");
        return;
    }

    rc = ela_send_friend_message(w, argv[0], argv[1], strlen(argv[1]) + 1);
    if (rc == 0)
        output("Send message successfully.\n");
    else
        output("Send message unsuccessfully(0x%x).\n", ela_get_error());
}

static void invite_response_callback(ElaCarrier *w, const char *friendid,
                                     int status, const char *reason,
                                     const char *data, size_t len, void *context)
{
    output("Got invite response from %s. ", friendid);
    if (status == 0 && g_mode == ACTIVE_MODE) {
        char *new_arg[1] = {NULL};
        char *add_stream_arg[2] = {NULL, NULL};

        output("message within response: %.*s\n", (int)len, data);

        session_init(w);
        output("Session initialized successfully.\n");

        new_arg[0] = (char*)friendid;
        session_new(w, 1, new_arg);
        output("Session created successfully.\n");

        add_stream_arg[0] = (char*)"plain";
        add_stream_arg[1] = (char*)"reliable";
        stream_add(w, 2, add_stream_arg);
    } else {
        output("refused: %s\n", reason);
    }
}

void invite_test_speed(const char *userid)
{
    int rc;

    if (userid == NULL) {
        output("Invalid invocation.\n");
        return;
    }

    rc = ela_invite_friend(g_carrier, userid, "hello", strlen("hello"),
                           invite_response_callback, NULL);
    if (rc == 0)
        output("Send invite testspeed request successfully.\n");
    else
        output("Send invite testspeed request unsuccessfully(0x%x).\n", ela_get_error());
}

static void reply_invite(ElaCarrier *w, int argc, char *argv[])
{
    int rc;
    int status = 0;
    const char *reason = NULL;
    const char *msg = NULL;
    size_t msg_len = 0;

    if (argc != 3) {
        output("Invalid invocation.\n");
        return;
    }

    if (strcmp(argv[1], "confirm") == 0) {
        msg = argv[2];
        msg_len = strlen(argv[2]);
    } else if (strcmp(argv[2], "refuse") == 0) {
        status = -1; // TODO: fix to correct status code.
        reason = argv[2];
    } else {
        output("Unknown sub command: %s\n", argv[1]);
        return;
    }

    rc = ela_reply_friend_invite(w, argv[0], status, reason, msg, msg_len);
    if (rc == 0)
        output("Send invite reply to inviter successfully.\n");
    else
        output("Send invite reply to inviter unsuccessfully(0x%x).\n", ela_get_error());
}

static void session_request_callback(ElaCarrier *w, const char *from,
            const char *sdp, size_t len, void *context)
{
    char *recv_arg[1] = {NULL};
    char *reply_arg[1] = {NULL};

    strncpy(session_ctx.remote_sdp, sdp, len);
    session_ctx.remote_sdp[len] = 0;
    session_ctx.sdp_len = len;

    output("Session request from[%s] with SDP:\n%s.\n", from, session_ctx.remote_sdp);

    recv_arg[0] = (char*)"start";
    stream_bulk_receive(w, 1, recv_arg);

    reply_arg[0] = (char*)"ok";
    session_reply_request(w, 1, reply_arg);
}

static void session_request_complete_callback(ElaSession *ws, int status,
                const char *reason, const char *sdp, size_t len, void *context)
{
    int rc;

    output("Session complete, status: %d, reason: %s\n", status,
           reason ? reason : "null");

    if (status != 0)
        return;

    strncpy(session_ctx.remote_sdp, sdp, len);
    session_ctx.remote_sdp[len] = 0;
    session_ctx.sdp_len = len;

    rc = ela_session_start(session_ctx.ws, session_ctx.remote_sdp,
                               session_ctx.sdp_len);

    output("Session start %s.\n", rc == 0 ? "successfully" : "unsuccessfully");
}

static void stream_bulk_receive(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 1) {
        output("Invalid invocation.\n");
        return;
    }

    if (strcmp(argv[0], "start") == 0) {
        session_ctx.bulk_mode = 1;
        session_ctx.bytes = 0;
        session_ctx.packets = 0;

        output("Ready to receive bulk data\n");
    } else if (strcmp(argv[0], "end") == 0) {
        struct timeval start = session_ctx.first_stamp;
        struct timeval end = session_ctx.last_stamp;
        int duration = 0;
        float speed = 0;

        duration = (int)((end.tv_sec - start.tv_sec) * 1000000 +
                             (end.tv_usec - start.tv_usec)) / 1000;
        duration = (duration == 0)  ? 1 : duration;
        speed = ((session_ctx.bytes / duration) * 1000) / 1024;

        output("\nFinish! Total %" PRIu64 " bytes in %d.%d seconds. %.2f KB/s\n",
               session_ctx.bytes,
               (int)(duration / 1000), (int)(duration % 1000), speed);

        session_ctx.bulk_mode = 0;
        session_ctx.bytes = 0;
        session_ctx.packets = 0;
    } else  {
        output("Invalid invocation.\n");
        return;
    }
}

static void stream_on_data(ElaSession *ws, int stream, const void *data,
                           size_t len, void *context)
{
    if (session_ctx.bulk_mode) {
        static int first_time = 0;
        static int trans_fd = 0;
        int ret = 0;
        JNIEnv *env = NULL;

        if (attach_to_vm(g_jvm, &env) == NULL) {
            output("Attach to vm unsuccessfully");
            return;
        }

        if (first_time == 0) {
            first_time = 1;
            trans_fd = open(g_received_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (trans_fd < 0) {
                output("stream_on_data open unsuccessfully.");
                detach_from_vm(g_jvm);
                return;
            }

            gettimeofday(&session_ctx.first_stamp, NULL);
            first_time = 1;
        } else {
            gettimeofday(&session_ctx.last_stamp, NULL);
        }

        session_ctx.bytes += len;
        session_ctx.packets++;
        ret = write(trans_fd, data, len);
        if (ret < len) {
            close(trans_fd);
            detach_from_vm(g_jvm);
            output("stream_on_data write unsuccessfully.");
            return;
        }

        if (session_ctx.bytes <= g_data_len) {
            jclass klass = (*env)->GetObjectClass(env, g_main_activity);
            jmethodID callback_id = (*env)->GetMethodID(env, klass, "updateProgress", "(II)V");

            if (callback_id == NULL) {
                output("update progress unsuccessfully\n");
            } else {
                int percent = g_bytes_transferred * 100 / g_data_len;
                (*env)->CallVoidMethod(env, g_main_activity, callback_id, g_data_len, len);
            }

            (*env)->DeleteLocalRef(env, klass);
        }

        if (session_ctx.bytes == g_data_len) {
            char *arg[2] = {NULL, NULL};
            char buf[64] = {0};
            char result[256] = {0};
            int ret = 0;

            close(trans_fd);

            strcpy(buf, MD5_CHECKSUM_MSG);
            strcpy(buf + strlen(buf), ":");
            ret = get_file_md5(g_received_file, buf + strlen(buf), sizeof(buf) - strlen(buf));
            if (ret < 0)
                output("passive end generated checksum unsuccessfully.");
            else {
                struct timeval start = {0};
                struct timeval end = {0};
                int duration = 0;
                float speed = 0;

                arg[0] = g_peer_id;
                arg[1] = buf;
                send_message((ElaCarrier*)context, 2, arg);

                if (session_ctx.packets == 1)
                    gettimeofday(&session_ctx.last_stamp, NULL);

                start = session_ctx.first_stamp;
                end = session_ctx.last_stamp;

                duration = (int)((end.tv_sec - start.tv_sec) * 1000000 +
                                     (end.tv_usec - start.tv_usec)) / 1000;
                duration = (duration == 0)  ? 1 : duration;
                speed = (session_ctx.bytes * 1000.0 / duration) / 1024;

                output("\n");
                sprintf(result, "Finish! Total %" PRIu64 " bytes in %d.%d seconds. %.2f KB/s",
                       session_ctx.bytes,
                       (int)(duration / 1000), (int)(duration % 1000), speed);
                output(result);
                output("\n");

                session_ctx.bulk_mode = 0;
                session_ctx.bytes = 0;
                session_ctx.packets = 0;

                jclass klass = (*env)->GetObjectClass(env, g_main_activity);
                jmethodID callback_id = (*env)->GetMethodID(env, klass, "showTestResult", "(Ljava/lang/String;)V");

                if (callback_id == NULL) {
                    output("show speed unsuccessfully\n");
                } else {
                    (*env)->CallVoidMethod(env, g_main_activity, callback_id, (*env)->NewStringUTF(env, result));
                }

                (*env)->DeleteLocalRef(env, klass);

                g_mode = INITIAL_MODE;
                memset(g_peer_id, 0, sizeof(g_peer_id));
                //pthread_exit(NULL);
            }
        }

        detach_from_vm(g_jvm);
    } else {
        output("Stream [%d] received data [%.*s]\n", stream, (int)len, (char*)data);
    }
}

static void stream_on_state_changed(ElaSession *ws, int stream,
        ElaStreamState state, void *context)
{
    const char *state_name[] = {
        "raw",
        "initialized",
        "transport_ready",
        "connecting",
        "connected",
        "deactivated",
        "closed",
        "failed"
    };

    output("Stream [%d] state changed to: %s\n", stream, state_name[state]);

    if (state == ElaStreamState_transport_ready)
        --session_ctx.unchanged_streams;

    if (state == ElaStreamState_initialized) {
        if (g_mode == ACTIVE_MODE) {
            session_request((ElaCarrier*)context);
        } else if (g_mode == PASSIVE_MODE) {
            char *reply_arg[3] = {NULL, NULL, NULL};

            reply_arg[0] = g_peer_id;
            reply_arg[1] = (char*)"confirm";
            reply_arg[2] = (char*)"ok";
            reply_invite((ElaCarrier*)context, 3, reply_arg);
        } else {
            output("Exception occurred");
        }
    } else if (state == ElaStreamState_connected) {
        if (g_mode == ACTIVE_MODE) {
            char *info_arg[1] = {NULL};
            char stream_id[32] = {0};
            char msg_len[32] = {0};
            char *msg_arg[2] = {NULL, NULL};
            char *write_arg[3] = {NULL, NULL, NULL};
            char buf[3][32] = {0};
            int pkt_size = 1000;
            int pkt_count = 1000;
            struct stat st = {0};
            int ret;

            sprintf(stream_id, "%d", g_stream_id);
            write_arg[0] = stream_id;
            stream_bulk_write((ElaCarrier*)context, 1, write_arg);
        } else {
            output("stream connected in passive mode\n");
        }
    } else {
        // Do nothing.
    }
}

static void session_init(ElaCarrier *w)
{
    int rc;

    rc = ela_session_init(w, session_request_callback, w);
    if (rc < 0) {
        output("Session initialized unsuccessfully.\n");
    }
    else {
        output("Session initialized successfully.\n");
    }
}

static void session_new(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 1) {
        output("Invalid invocation.\n");
        return;
    }

    session_ctx.ws = ela_session_new(w, argv[0]);
    if (!session_ctx.ws) {
        output("Create session unsuccessfully.\n");
    } else {
        output("Create session successfully.\n");
    }
    session_ctx.unchanged_streams = 0;
}

static void stream_add(ElaCarrier *w, int argc, char *argv[])
{
    int rc;
    int options = 0;
    ElaStreamCallbacks callbacks;

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.state_changed = stream_on_state_changed;
    callbacks.stream_data = stream_on_data;

    if (argc < 1) {
        output("Invalid invocation.\n");
        return;
    } else if (argc > 1) {
        int i;

        for (i = 0; i < argc; i++) {
            if (strcmp(argv[i], "reliable") == 0) {
                options |= ELA_STREAM_RELIABLE;
            } else if (strcmp(argv[i], "plain") == 0) {
                options |= ELA_STREAM_PLAIN;
            } else {
                output("Invalid invocation.\n");
                return;
            }
        }
    }

    rc = ela_session_add_stream(session_ctx.ws, ElaStreamType_text,
                                options, &callbacks, w);
    if (rc < 0) {
        output("Add stream unsuccessfully.\n");
    }
    else {
        session_ctx.unchanged_streams++;
        output("Add stream successfully and stream id %d.\n", rc);
        g_stream_id = rc;
    }
}

static void session_request(ElaCarrier *w)
{
    int rc;

    rc = ela_session_request(session_ctx.ws,
                             session_request_complete_callback, w);
    if (rc < 0) {
        output("session request unsuccessfully.\n");
    }
    else {
        output("session request successfully.\n");
    }
}

static void session_reply_request(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if ((argc != 1) && (argc != 2)) {
        output("Invalid invocation.\n");
        return;
    }

    if ((strcmp(argv[0], "ok") == 0) && (argc == 1)) {
        rc = ela_session_reply_request(session_ctx.ws, 0, NULL);
        if (rc < 0) {
            output("response invite unsuccessfully.\n");
        } else {
            output("response invite successfully.\n");

            while (session_ctx.unchanged_streams > 0)
                usleep(200);

            rc = ela_session_start(session_ctx.ws, session_ctx.remote_sdp,
                                       session_ctx.sdp_len);

            output("Session start %s.\n", rc == 0 ? "successfully" : "unsuccessfully");
        }
    } else if ((strcmp(argv[0], "refuse") == 0) && (argc == 2)) {
        rc = ela_session_reply_request(session_ctx.ws, 1, argv[2]);
        if (rc < 0) {
            output("response invite unsuccessfully.\n");
        } else {
            output("response invite successfully.\n");
        }
    } else {
        output("Invalid response.\n");
        return;
    }
}

struct bulk_write_args {
    int stream;
    int packet_size;
    int packet_count;
};
struct bulk_write_args args;

static void *bulk_write_thread(void *arg)
{
    ssize_t rc;
    char packet[1024] = {0};
    char stream_id[32] = {0};
    char msg_content[64] = {0};
    char *msg_arg[2] = {0};
    char *info_arg[1] = {0};
    char result[256] = {0};
    struct bulk_write_args *args = (struct bulk_write_args *)arg;
    struct timeval start, end;
    int duration;
    float speed;
    JNIEnv *env = NULL;
    AAssetManager *mgr = NULL;
    AAsset *asset = NULL;
    MD5_CTX md5_ctx = {0};
    jclass klass;
    jmethodID callback_id;

    if (attach_to_vm(g_jvm, &env) == NULL)
        return NULL;

    mgr = AAssetManager_fromJava(env, g_asset_manager);
    if (mgr == NULL) {
        detach_from_vm(g_jvm);
        return NULL;
    }

    asset = AAssetManager_open(mgr, g_transferred_file, AASSET_MODE_UNKNOWN);
    if (asset == NULL) {
        detach_from_vm(g_jvm);
        return NULL;
    }

    g_data_len = AAsset_getLength64(asset);
    msg_arg[0] = g_peer_id;
    sprintf(msg_content, "%s:%d", DATA_LEN_MSG, g_data_len);
    msg_arg[1] = msg_content;
    send_message(g_carrier, 2, msg_arg);

    sprintf(stream_id, "%d", g_stream_id);
    info_arg[0] = stream_id;
    stream_get_info(g_carrier, 1, info_arg);
    output("Begin sending data");
    gettimeofday(&start, NULL);
    init_md5(&md5_ctx);

    while ((rc = AAsset_read(asset, packet, sizeof(packet))) > 0) {
        rc = ela_stream_write(session_ctx.ws, args->stream,
                                  packet, rc);
        if (rc == 0) {
            usleep(100);
            continue;
        } else if (rc < 0) {
            if (ela_get_error() == ELA_GENERAL_ERROR(ELAERR_BUSY)) {
                usleep(100);
                continue;
            } else {
                output("\nWrite data unsuccessfully.\n");
                output("error no.:%x", ela_get_error());
                detach_from_vm(g_jvm);
                return NULL;
            }
        } else {
            g_bytes_transferred += rc;
            update_md5(&md5_ctx, packet, rc);

            if (g_bytes_transferred <= g_data_len) {
                jclass klass = (*env)->GetObjectClass(env, g_main_activity);
                jmethodID callback_id = (*env)->GetMethodID(env, klass, "updateProgress", "(II)V");

                if (callback_id == NULL) {
                    output("update progress unsuccessfully\n");
                } else {
                    int percent = g_bytes_transferred * 100 / g_data_len;
                    (*env)->CallVoidMethod(env, g_main_activity, callback_id, g_data_len, rc);
                }

                (*env)->DeleteLocalRef(env, klass);
            }
        }
    }

    get_bytes_md5(&md5_ctx, g_md5, sizeof(g_md5));
    gettimeofday(&end, NULL);

    duration = (int)((end.tv_sec - start.tv_sec) * 1000000 +
                     (end.tv_usec - start.tv_usec)) / 1000;
    duration = (duration == 0) ? 1 : duration;
    speed = (g_data_len * 1000.0 / duration) / 1024;

    output("\n");
    sprintf(result, "Finish! Total %" PRIu64 " bytes in %d.%d seconds. %.2f KB/s",
           g_data_len,
           (int)(duration / 1000), (int)(duration % 1000), speed);
    output(result);
    output("\n");

    klass = (*env)->GetObjectClass(env, g_main_activity);
    callback_id = (*env)->GetMethodID(env, klass, "showTestResult", "(Ljava/lang/String;)V");

    if (callback_id == NULL) {
        output("show speed unsuccessfully\n");
    } else {
        (*env)->CallVoidMethod(env, g_main_activity, callback_id, (*env)->NewStringUTF(env, result));
    }

    (*env)->DeleteLocalRef(env, klass);
    detach_from_vm(g_jvm);

    return NULL;
}

static void stream_bulk_write(ElaCarrier *w, int argc, char *argv[])
{
    pthread_attr_t attr;
    pthread_t th;

    if (argc != 1) {
        output("Invalid invocation.\n");
        return;
    }

    args.stream = atoi(argv[0]);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&th, &attr, bulk_write_thread, &args);
    pthread_join(th, NULL);
    pthread_attr_destroy(&attr);
}

static void stream_get_info(ElaCarrier *w, int argc, char *argv[])
{
    int rc;
    ElaTransportInfo info;

    const char *topology_name[] = {
        "LAN",
        "P2P",
        "RELAYED"
    };

    const char *addr_type[] = {
        "HOST   ",
        "SREFLEX",
        "PREFLEX",
        "RELAY  "
    };

    if (argc != 1) {
        output("Invalid invocation.\n");
        return;
    }

    rc = ela_stream_get_transport_info(session_ctx.ws, atoi(argv[0]), &info);
    if (rc < 0) {
        output("get remote addr unsuccessfully.\n");
        return;
    }

    output("Stream transport information:\n");
    output("    Network: %s\n", topology_name[info.topology]);
    output("      Local: %s %s:%d", addr_type[info.local.type], info.local.addr, info.local.port);

    if (*info.local.related_addr)
        output(" related %s:%d\n", info.local.related_addr, info.local.related_port);
    else
        output("\n");

    output("     Remote: %s %s:%d", addr_type[info.remote.type], info.remote.addr, info.remote.port);

    if (*info.remote.related_addr)
        output(" related %s:%d\n", info.remote.related_addr, info.remote.related_port);
    else
        output("\n");
}

static void connection_callback(ElaCarrier *w, ElaConnectionStatus status,
                                void *context)
{
    JNIEnv  *env = g_env;

    switch (status) {
    case ElaConnectionStatus_Connected:
        output("Connected to carrier network.\n");
        break;

    case ElaConnectionStatus_Disconnected:
        output("Disconnected from carrier network.\n");
        break;

    default:
        output("Error!!! Got unknown connection status %d.\n", status);
    }

    if (env == NULL) {
        output("Got env unsuccessfully in %s\n", __func__);
    } else {
        jclass klass = (*env)->GetObjectClass(env, g_main_activity);
        jmethodID callback_id = (*env)->GetMethodID(env, klass, "OnConnectionChanged", "(Z)V");

        if (callback_id == NULL) {
            output("invoked callback unsuccessfully in connection_callback\n");
        } else {
            jboolean online = (status == ElaConnectionStatus_Connected) ? true : false;
            (*env)->CallVoidMethod(env, g_main_activity, callback_id, online);
        }
        (*env)->DeleteLocalRef(env, klass);
    }
}

static void friend_info_callback(ElaCarrier *w, const char *friendid,
                                 const ElaFriendInfo *info, void *context)
{
    output("Friend information changed:\n");
    display_friend_info(info);
}

static void friend_connection_callback(ElaCarrier *w, const char *friendid,
                                       ElaConnectionStatus status, void *context)
{
    JNIEnv *env = g_env;

    switch (status) {
    case ElaConnectionStatus_Connected:
        output("Friend[%s] connection changed to be online\n", friendid);
        break;

    case ElaConnectionStatus_Disconnected:
        output("Friend[%s] connection changed to be offline.\n", friendid);
        break;

    default:
        output("Error!!! Got unknown connection status %d.\n", status);
    }

    if (env == NULL) {
        output("Got env unsuccessfully in %s\n", __func__);
    } else {
        jclass klass = (*env)->GetObjectClass(env, g_main_activity);
        jmethodID callback_id = (*env)->GetMethodID(env, klass, "OnFriendConnectionChanged", "(Ljava/lang/String;Z)V");

        if (callback_id == NULL) {
            output("invoked callback unsuccessfully in friend_connection_callback\n");
        } else {
            jboolean online = (status == ElaConnectionStatus_Connected) ? true : false;
            (*env)->CallVoidMethod(env, g_main_activity, callback_id, (*env)->NewStringUTF(env, friendid), online);
        }
        (*env)->DeleteLocalRef(env, klass);
    }
}

static void friend_presence_callback(ElaCarrier *w, const char *friendid,
                                     ElaPresenceStatus status,
                                     void *context)
{
    if (status >= ElaPresenceStatus_None &&
        status <= ElaPresenceStatus_Busy) {
        output("Friend[%s] change presence to %s\n", friendid, presence_name[status]);
    } else {
        output("Error!!! Got unknown presence status %d.\n", status);
    }
}

static void friend_request_callback(ElaCarrier *w, const char *userid,
                                    const ElaUserInfo *info, const char *hello,
                                    void *context)
{
    char *arg[1] = {NULL};
    int ret = 0;
    JNIEnv *env = g_env;

    output("Friend request from user[%s] with HELLO: %s.\n",
           *info->name ? info->name : userid, hello);

    if (env == NULL) {
        output("Got env unsuccessfully in %s\n", __func__);
    } else {
        jclass klcass = (*env)->GetObjectClass(env, g_main_activity);
        jmethodID callback_id = (*env)->GetMethodID(env, klcass, "OnFriendRequest", "(Ljava/lang/String;Ljava/lang/String;)V");

        if (callback_id == NULL) {
            output("invoked callback unsuccessfully in friend_request_callback\n");
        } else {
            (*env)->CallVoidMethod(env, g_main_activity, callback_id, (*env)->NewStringUTF(env, userid), (*env)->NewStringUTF(env, hello));
        }
        (*env)->DeleteLocalRef(env, klcass);
    }
}

static char **strsplit(const char* str, const char* delim, size_t* numtokens) {
    // copy the original string so that we don't overwrite parts of it
    // (don't do this if you don't need to keep the old line,
    // as this is less efficient)
    char *s = strdup(str);
    // these three variables are part of a very common idiom to
    // implement a dynamically-growing array
    size_t tokens_alloc = 1;
    size_t tokens_used = 0;
    char **tokens = calloc(tokens_alloc, sizeof(char*));
    char *token, *strtok_ctx;

    for (token = strtok_r(s, delim, &strtok_ctx);
         token != NULL;
         token = strtok_r(NULL, delim, &strtok_ctx)) {
        // check if we need to allocate more space for tokens
        if (tokens_used == tokens_alloc) {
            tokens_alloc *= 2;
            tokens = realloc(tokens, tokens_alloc * sizeof(char*));
        }
        tokens[tokens_used++] = strdup(token);
    }

    // cleanup
    if (tokens_used == 0) {
        free(tokens);
        tokens = NULL;
    } else {
        tokens = realloc(tokens, tokens_used * sizeof(char*));
    }

    *numtokens = tokens_used;
    free(s);

    return tokens;
}

static void message_callback(ElaCarrier *w, const char *from,
                             const char *msg, size_t len, void *context)
{
    output("Message from friend[%s]: %.*s\n", from, (int)len, msg);

    int token_cnt = 0;
    char **tokens = strsplit(msg, ":", &token_cnt);

    if (token_cnt != 2) {
        free(tokens);
        return;
    }

    if (g_mode == INITIAL_MODE) {
        if (strcmp(tokens[0], TEST_SPEED_MSG) == 0) {
            // todo: invoke MainActivity callback.
            char *arg[1] = {NULL};
            int ret = 0;
            JNIEnv *env = g_env;

            if (env == NULL) {
                output("Got env unsuccessfully in %s\n", __func__);
            } else {
                jclass klass = (*env)->GetObjectClass(env, g_main_activity);
                jmethodID callback_id = (*env)->GetMethodID(env, klass, "OnFriendMessage", "(Ljava/lang/String;Ljava/lang/String;)V");

                if (callback_id == NULL) {
                    output("invoked callback unsuccessfully in message_callback\n");
                } else {
                    (*env)->CallVoidMethod(env, g_main_activity, callback_id, (*env)->NewStringUTF(env, from),
                                           (*env)->NewStringUTF(env, tokens[1]));
                }

                (*env)->DeleteLocalRef(env, klass);
            }
        } else if (strcmp(tokens[0], TEST_SPEED_ACK_MSG) == 0) {
            if (strcmp(tokens[1], ACCEPT_TEST_SPEED) == 0) {
                g_mode = ACTIVE_MODE;
                strcpy(g_peer_id, from);
                invite_test_speed(from);
            } else if (strcmp(tokens[1], REFUSE_TEST_SPEED) == 0) {
                char *arg[1] = {NULL};
                int ret = 0;
                JNIEnv *env = g_env;

                if (env == NULL) {
                    output("Got env unsuccessfully in %s\n", __func__);
                } else {
                    jclass klass = (*env)->GetObjectClass(env, g_main_activity);
                    jmethodID callback_id = (*env)->GetMethodID(env, klass, "OnFriendMessage", "(Ljava/lang/String;Ljava/lang/String;)V");

                    if (callback_id == NULL) {
                        output("invoked callback unsuccessfully in message_callback\n");
                    } else {
                        (*env)->CallVoidMethod(env, g_main_activity, callback_id, (*env)->NewStringUTF(env, from),
                                               (*env)->NewStringUTF(env, "refuse"));
                    }

                    (*env)->DeleteLocalRef(env, klass);
                }
            } else {
                // undefined acknowledgement

            }
        }
    } else if (g_mode == PASSIVE_MODE) {
        if (strcmp(tokens[0], "data_len") == 0) {
            g_data_len = atoi(tokens[1]);
            output("Got data length: %d.\n", g_data_len);
        } else if (strcmp(tokens[0], "test_speed") == 0) {
            char *arg[1] = {NULL};
            int ret = 0;
            JNIEnv *env = g_env;

            if (env == NULL) {
                output("Got env unsuccessfully in %s\n", __func__);
            } else {
                jclass klass = (*env)->GetObjectClass(env, g_main_activity);
                jmethodID callback_id = (*env)->GetMethodID(env, klass, "OnFriendMessage", "(Ljava/lang/String;Ljava/lang/String;)V");

                if (callback_id == NULL) {
                    output("invoked callback unsuccessfully in message_callback\n");
                } else {
                    (*env)->CallVoidMethod(env, g_main_activity, callback_id, (*env)->NewStringUTF(env, from), (*env)->NewStringUTF(env, tokens[1]));
                }

                (*env)->DeleteLocalRef(env, klass);
            }
        } else {
            output("No response");
        }
    } else { //ACTIVE_MODE
        if (strcmp(tokens[0], MD5_CHECKSUM_MSG) == 0) {
            output("Got md5 checksum from peer:%s\n", tokens[1]);

            if (strcasecmp(g_md5, tokens[1]) == 0) {
                output("Sent data successfully!\n");
            } else {
                output("Sent data unsuccessfully!\n");
            }

            g_mode = INITIAL_MODE;
            memset(g_peer_id, 0, sizeof(g_peer_id));
        } else {
            output("No response");
        }
        //exit(0);
    }

    free(tokens);
}

static void invite_request_callback(ElaCarrier *w, const char *from,
                                    const char *data, size_t len, void *context)
{
    char *new_arg[1] = {NULL};
    char *add_stream_arg[2] = {NULL, NULL};

    output("Invite request from[%s] with data: %.*s\n", from, (int)len, data);

    strcpy(g_peer_id, from);
    session_init(w);

    new_arg[0] = (char*)from;
    session_new(w, 1, new_arg);

    add_stream_arg[0] = (char*)"plain";
    add_stream_arg[1] = (char*)"reliable";
    stream_add(w, 2, add_stream_arg);
}

static void ready_callback(ElaCarrier *w, void *context)
{
    output("ready_callback invoked\n");
}

#include <sys/resource.h>

int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}

void signal_handler(int signum)
{
    exit(-1);
}

int start_carrier(const char *file_transferred, const char *file_received)
{
    char buffer[2048] = {0};
    ElaOptions opts = {0};
    int wait_for_attach = 0;
    char buf[ELA_MAX_ADDRESS_LEN+1];
    ElaCallbacks callbacks = {0};
    int rc;
    int i;
    bool enable_udp = true;
    int log_level = 4;
    const char *log_file = NULL;
    const char *data_dir = "/sdcard/speedtestdata";

    if (file_transferred == NULL || file_received == NULL)
        return -1;

    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGSEGV, signal_handler);

    sys_coredump_set(true);
    memset(&opts, 0, sizeof(opts));
    strcpy(g_transferred_file, file_transferred);
    strcpy(g_received_file, file_received);

    if (wait_for_attach) {
        printf("Wait for debugger attaching, process id is: %d.\n", getpid());
        printf("After debugger attached, press any key to continue......");
        getchar();
    }

    if (!*g_transferred_file) {
        fprintf(stdout, "Transferred file name was no specified!\n");
        return -1;
    }

    ela_log_init(log_level, log_file, log_print);

    opts.udp_enabled = enable_udp;
    opts.persistent_location = data_dir;
    opts.bootstraps_size = 5;
    opts.bootstraps = (BootstrapNode *)calloc(1, sizeof(BootstrapNode) * opts.bootstraps_size);
    if (!opts.bootstraps) {
        fprintf(stderr, "out of memory.");
        return -1;
    }

    opts.bootstraps[0].ipv4 = "13.58.208.50";
    opts.bootstraps[0].port = "33445";
    opts.bootstraps[0].public_key = "89vny8MrKdDKs7Uta9RdVmspPjnRMdwMmaiEW27pZ7gh";
    opts.bootstraps[1].ipv4 = "18.216.102.47";
    opts.bootstraps[1].port = "33445";
    opts.bootstraps[1].public_key = "G5z8MqiNDFTadFUPfMdYsYtkUDbX5mNCMVHMZtsCnFeb";
    opts.bootstraps[2].ipv4 = "18.216.6.197";
    opts.bootstraps[2].port = "33445";
    opts.bootstraps[2].public_key = "H8sqhRrQuJZ6iLtP2wanxt4LzdNrN2NNFnpPdq1uJ9n2";
    opts.bootstraps[3].ipv4 = "54.223.36.193";
    opts.bootstraps[3].port = "33445";
    opts.bootstraps[3].public_key = "5tuHgK1Q4CYf4K5PutsEPK5E3Z7cbtEBdx7LwmdzqXHL";
    opts.bootstraps[4].ipv4 = "52.80.187.125";
    opts.bootstraps[4].port = "33445";
    opts.bootstraps[4].public_key = "3khtxZo89SBScAMaHhTvD68pPHiKxgZT6hTCSZZVgNEm";

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.connection_status = connection_callback;
    callbacks.friend_list = friends_list_callback;
    callbacks.friend_connection = friend_connection_callback;
    callbacks.friend_info = friend_info_callback;
    callbacks.friend_presence = friend_presence_callback;
    callbacks.friend_request = friend_request_callback;
    callbacks.friend_added = friend_added_callback;
    callbacks.friend_removed = friend_removed_callback;
    callbacks.friend_message = message_callback;
    callbacks.friend_invite = invite_request_callback;
    callbacks.ready = ready_callback;

    ela_clear_error();
    g_carrier = ela_new(&opts, &callbacks, NULL);
    free(opts.bootstraps);

    if (!g_carrier) {
        int error_code = ela_get_error();
        output("Error create carrier instance: 0x%x\n", ela_get_error());
        output("Press any key to quit...");
        goto quit;
    }

    output("Carrier node identities:\n");
    output("   Node ID: %s\n", ela_get_nodeid(g_carrier, buf, sizeof(buf)));
    output("   User ID: %s\n", ela_get_userid(g_carrier, buf, sizeof(buf)));
    output("   Address: %s\n\n", ela_get_address(g_carrier, buf, sizeof(buf)));
    output("\n");

    rc = ela_run(g_carrier, 10);
    if (rc != 0) {
        output("Error start carrier loop: 0x%x\n", ela_get_error());
        output("Press any key to quit...");
        ela_kill(g_carrier);
        goto quit;
    }

quit:
    return 0;
}

void stop_carrier()
{
    if (g_carrier)
        ela_kill(g_carrier);
}

void request_test_speed(const char *userid)
{
    int rc;
    char msg[32] = {0};

    if (userid == NULL) {
        output("Invalid invocation.\n");
        return;
    }

    sprintf(msg, "%s:%s", TEST_SPEED_MSG, "hello");
    rc = ela_send_friend_message(g_carrier, userid, msg, strlen(msg) + 1);
    if (rc == 0)
        output("Send message successfully.\n");
    else
        output("Send message unsuccessfully(0x%x).\n", ela_get_error());
}

void accept_test_speed(const char *userid)
{
    int rc;
    char msg[32] = {0};

    if (userid == NULL) {
        output("Invalid invocation.\n");
        return;
    }

    g_mode = PASSIVE_MODE;
    sprintf(msg, "%s:%s", TEST_SPEED_ACK_MSG, "accept");
    rc = ela_send_friend_message(g_carrier, userid, msg, strlen(msg) + 1);
    if (rc == 0) {
        output("Send message successfully.\n");
    } else {
        output("Send message unsuccessfully(0x%x).\n", ela_get_error());
        g_mode = INITIAL_MODE;
    }
}

void refuse_test_speed(const char *userid)
{
    int rc;
    char msg[32] = {0};

    if (userid == NULL) {
        output("Invalid invocation.\n");
        return;
    }

    sprintf(msg, "%s:%s", TEST_SPEED_ACK_MSG, "refuse");
    rc = ela_send_friend_message(g_carrier, userid, msg, strlen(msg) + 1);
    if (rc == 0)
        output("Send message successfully.\n");
    else
        output("Send message unsuccessfully(0x%x).\n", ela_get_error());
}
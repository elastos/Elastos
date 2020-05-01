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
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#include <pthread.h>

#include <crystal.h>

#include "ela_carrier.h"
#include "ela_session.h"
#include "ela_filetransfer.h"

#include "test_context.h"
#include "test_helper.h"
#include "config.h"
#include "cmd.h"
#include "robot.h"
#include "carrier_extension.h"

static ElaFileTransferInfo file_transfer_info;
static ElaFileTransferCallbacks file_transfer_cbs;

static CarrierContextExtra extra = {
    .tid = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .userid = {0},
    .bundle = NULL,
    .data   = NULL,
    .len    = 0,
    .test_offmsg = OffMsgCase_Zero,
    .test_offmsg_count = 0,
    .expected_offmsg_count = 0,
    .test_offmsg_expires = {0},
    .offmsg_header = {0},
    .gcookie = {0},
    .gcookie_len = 0,
    .gfrom  = {0},
    .groupid = {0},
    .fileid = {0},
    .recv_file = {"ReceivedFile"}
};

static void print_user_info(const ElaUserInfo* info)
{
    vlogD("       userid: %s", info->userid);
    vlogD("         name: %s", info->name);
    vlogD("  description: %s", info->description);
    vlogD("   has_avatar: %s", info->has_avatar ? "true" : "false");
    vlogD("       gender: %s", info->gender);
    vlogD("        phone: %s", info->phone);
    vlogD("        email: %s", info->email);
    vlogD("       region: %s", info->region);
}

void print_friend_info(const ElaFriendInfo* info, int order)
{
    if (order > 0)
        vlogD(" friend %d:", order);

    print_user_info(&info->user_info);
    vlogD("        label: %s", info->label);
    vlogD("     presence: %d", info->presence);
}

static void idle_cb(ElaCarrier *w, void *context)
{
    CarrierContextExtra *extra = ((TestContext*)context)->carrier->extra;
    struct timeval now;

    pthread_mutex_lock(&extra->mutex);
    if (extra->test_offmsg == OffMsgCase_Single) {
        gettimeofday(&now, NULL);
        if (timercmp(&now, &extra->test_offmsg_expires, >)) {
            write_ack("offmsglost\n");
            extra->test_offmsg = OffMsgCase_Zero;
        }
    } else if(extra->test_offmsg == OffMsgCase_Bulk) {
        gettimeofday(&now, NULL);
        if (timercmp(&now, &extra->test_offmsg_expires, >)) {
            write_ack("%d\n", extra->test_offmsg_count);
            extra->test_offmsg = OffMsgCase_Zero;
            extra->test_offmsg_count = 0;
            extra->expected_offmsg_count = 0;
        }
    }
    pthread_mutex_unlock(&extra->mutex);
}

static void connection_status_cb(ElaCarrier *w, ElaConnectionStatus status,
                                 void *context)
{
    vlogD("Robot connection status changed -> %s", connection_str(status));
}

static void ready_cb(ElaCarrier *w, void *context)
{
    char address[ELA_MAX_ADDRESS_LEN + 1];
    char robotid[ELA_MAX_ID_LEN + 1];

    ela_get_userid(w, robotid, sizeof(robotid));
    ela_get_address(w, address, sizeof(address));

    vlogI("Robot is ready");
    write_ack("ready %s %s\n", robotid, address);
}

static
void self_info_cb(ElaCarrier *w, const ElaUserInfo *info, void *context)
{
    vlogD("Received current user information:");
    print_user_info(info);
}

static
bool friend_list_cb(ElaCarrier* w, const ElaFriendInfo *info, void *context)
{
    static bool grouped = false;
    static int idx = 1;

    if (info) {
        if (!grouped) {
            vlogD("Received friend list and listed below:");
            grouped = true;
        }
        print_friend_info(info, idx);
        idx++;

    } else {
        if (grouped) {
            grouped = false;
            idx = 1;
        }
    }

    return true;
}

static void friend_connection_cb(ElaCarrier *w, const char *friendid,
                                 ElaConnectionStatus status, void *context)
{
    CarrierContext *wctx = ((TestContext *)context)->carrier;

    vlogD("Friend %s's connection status changed -> %s",
          friendid, connection_str(status));

    status_cond_signal(wctx->friend_status_cond, status);
}

static void friend_info_cb(ElaCarrier *w, const char *friendid,
                          const ElaFriendInfo *info, void *context)
{
    vlogD("Friend %s's information changed ->", friendid);
    print_friend_info(info, 0);
}

static const char *presence_name[] = {
    "None",
    "Away",
    "Busy"
};

static void friend_presence_cb(ElaCarrier *w, const char *friendid,
                               ElaPresenceStatus status, void *context)
{
    vlogI("Friend %s's presence changed -> %s", friendid,
          presence_name[status]);
}

static void* ela_accept_friend_entry(void *arg)
{
    TestContext *ctx = (TestContext *)arg;
    CarrierContext *wctx = ctx->carrier;
    char *argv[]= {
        "faccept",
         wctx->extra->userid
    };

    faccept(ctx, 2, argv);
    return NULL;
}

static void friend_request_cb(ElaCarrier *w, const char *userid,
                const ElaUserInfo *info, const char *hello, void *context)
{
    TestContext *ctx = (TestContext *)context;
    CarrierContext *wctx = ctx->carrier;

    vlogD("Received friend request from user %s", userid);
    print_user_info(info);
    vlogD("  hello: %s", hello);

    if (!strcmp(hello, "auto-reply")) {
        pthread_t tid;

        strcpy(wctx->extra->userid, userid);
        pthread_create(&tid, 0, &ela_accept_friend_entry, ctx);
        pthread_detach(tid);
    } else {
        write_ack("hello %s\n", hello);
    }
}

static void friend_added_cb(ElaCarrier *w, const ElaFriendInfo *info,
                            void *context)
{
    CarrierContext *wctx = ((TestContext *)context)->carrier;

    vlogI("New friend %s added", info->user_info.userid);
    print_friend_info(info, 0);
    cond_signal(wctx->cond);
}

static void friend_removed_cb(ElaCarrier* w, const char* friendid, void *context)
{
    CarrierContext *wctx = ((TestContext *)context)->carrier;

    vlogI("Friend %s is removed", friendid);
    cond_signal(wctx->cond);
}

static void friend_message_cb(ElaCarrier *w, const char *from,
                              const void *msg, size_t len,
                              int64_t timestamp, bool is_offline,
                              void *context)
{
    CarrierContextExtra *extra = ((TestContext*)context)->carrier->extra;

    vlogD("Received %s message from %s", is_offline ? "offline" : "online", from);
    vlogD(" msg: (%d) %.*s", len, len, (const char *)msg);

    pthread_mutex_lock(&extra->mutex);
    if (is_offline && extra->test_offmsg == OffMsgCase_Single) {
        if (strstr((const char*)msg, extra->offmsg_header)) {
            write_ack("%.*s\n", len, msg);
            extra->test_offmsg = OffMsgCase_Zero;
        }
    } else if (is_offline && extra->test_offmsg == OffMsgCase_Bulk) {
        if (strstr((const char*)msg, extra->offmsg_header)) {
            extra->test_offmsg_count++;
            if (extra->test_offmsg_count == extra->expected_offmsg_count) {
                write_ack("%d\n", extra->test_offmsg_count);
                extra->test_offmsg = OffMsgCase_Zero;
                extra->test_offmsg_count = 0;
                extra->expected_offmsg_count = 0;
            }
        }
    } else {
        if (!is_offline)
            write_ack("%.*s\n", len, msg);
    }
    pthread_mutex_unlock(&extra->mutex);
}

static void friend_invite_cb(ElaCarrier *w, const char *from, const char *bundle,
                             const void *data, size_t len, void *context)
{
    CarrierContextExtra *extra = ((TestContext*)context)->carrier->extra;

    vlogD("Recevied friend invite from %s", from);
    vlogD(" data: %.*s", len, (const char *)data);

    if (bundle)
        extra->bundle = strdup(bundle);

    if (len <= ELA_MAX_APP_MESSAGE_LEN)
        write_ack("data %.*s\n", len, data);
    else {
        extra->data = (char*)malloc(len);
        memcpy(extra->data, data, len);
        extra->len = (int)len;
        write_ack("data bigdata %d\n", (int)len);
    }
}

static void group_invite_cb(ElaCarrier *w, const char *from,
                            const void *cookie, size_t len, void *context)
{
    TestContext *ctx = (TestContext *)context;
    CarrierContext *wctx = ctx->carrier;

    memcpy(wctx->extra->gcookie, cookie, len);
    wctx->extra->gcookie_len = (int)len;
    strcpy(wctx->extra->gfrom, from);

    write_ack("ginvite received\n");
}

static void group_connected_cb(ElaCarrier *carrier, const char *groupid,
                               void *context)
{
    TestContext *ctx = (TestContext *)context;
    CarrierContext *wctx = ctx->carrier;

    assert(!strcmp(groupid, wctx->extra->groupid));

    cond_signal(wctx->group_cond);
}

static void group_message_cb(ElaCarrier *carrier, const char *groupid,
                      const char *from, const void *message, size_t length,
                      void *context)
{
    TestContext *ctx = (TestContext *)context;
    CarrierContext *wctx = ctx->carrier;

    assert(!strcmp(groupid, wctx->extra->groupid));

    write_ack("gmsg %.*s\n", length, message);
}

static void group_title_cb(ElaCarrier *carrier, const char *groupid,
                        const char *from, const char *title, void *context)
{
    write_ack("gtitle %s\n", title);
}

static void peer_list_changed_cb(ElaCarrier *carrier, const char *groupid,
                                 void *context)
{
    TestContext *ctx = (TestContext *)context;
    CarrierContext *wctx = ctx->carrier;

    cond_signal(wctx->group_cond);
}

static ElaCallbacks callbacks = {
    .idle            = idle_cb,
    .connection_status = connection_status_cb,
    .ready           = ready_cb,
    .self_info       = self_info_cb,
    .friend_list     = friend_list_cb,
    .friend_connection = friend_connection_cb,
    .friend_info     = friend_info_cb,
    .friend_presence = friend_presence_cb,
    .friend_request  = friend_request_cb,
    .friend_added    = friend_added_cb,
    .friend_removed  = friend_removed_cb,
    .friend_message  = friend_message_cb,
    .friend_invite   = friend_invite_cb,
    .group_invite    = group_invite_cb,
    .group_callbacks = {
        .group_connected = group_connected_cb,
        .group_message   = group_message_cb,
        .group_title     = group_title_cb,
        .peer_list_changed = peer_list_changed_cb
    }
};

static void ft_state_changed_cb(ElaFileTransfer *filetransfer,
                                FileTransferConnection state, void *context)
{

}

static void ft_file_cb(ElaFileTransfer *filetransfer, const char *fileid,
                      const char *filename, uint64_t size, void *context)
{
    TestContext *ctx = (TestContext *)context;
    CarrierContext *wctx = ctx->carrier;
    CarrierContextExtra *extra = wctx->extra;

    write_ack("%s %s %d\n", filename, fileid, size);
    strcpy(extra->fileid, fileid);
}

static void ft_pull_cb(ElaFileTransfer *filetransfer, const char *fileid,
                       uint64_t offset, void *context)
{

}

static bool ft_data_cb(ElaFileTransfer *filetransfer, const char *fileid,
                       const uint8_t *data, size_t length, void *context)
{
    const uint8_t *ack_data = data;

    if (length == ELA_MAX_USER_DATA_LEN - 1)
        ack_data = (const uint8_t*)"bigdata";

    if (data == NULL)
        ack_data = (const uint8_t*)"null";

    write_ack("ft_data %s %s %d\n", fileid, ack_data, length);
    return true;
}

static void ft_pending_cb(ElaFileTransfer *filetransfer, const char *fileid,
                          void *context)
{

}

static void ft_resume_cb(ElaFileTransfer *filetransfer, const char *fileid,
                         void *context)
{

}

static void ft_cancel_cb(ElaFileTransfer *filetransfer, const char *fileid,
                         int status, const char *reason, void *context)
{

}

static ElaFileTransferCallbacks ft_cbs = {
    .state_changed = ft_state_changed_cb,
    .file = ft_file_cb,
    .pull = ft_pull_cb,
    .data = ft_data_cb,
    .pending = ft_pending_cb,
    .resume = ft_resume_cb,
    .cancel = ft_cancel_cb
};

static ElaFileTransferInfo ft_info = {
    .filename = "robotfile",
    .fileid = {0},
    .size = 1
};

static StatusCondition DEFINE_STATUS_COND(friend_status_cond);
static Condition DEFINE_COND(cond);
static Condition DEFINE_COND(group_cond);

CarrierContext carrier_context = {
    .cbs = &callbacks,
    .carrier = NULL,
    .ft = NULL,
    .ft_info = &ft_info,
    .ft_cbs = &ft_cbs,
    .ft_cond = NULL,
    .cond = &cond,
    .friend_status_cond = &friend_status_cond,
    .group_cond = &group_cond,
    .extra = &extra
};

static
void ext_invite_callback(ElaCarrier *carrier, const char *from,
                         const void *data, size_t len, void *context)
{
    CarrierContextExtra *extra = ((TestContext*)context)->carrier->extra;

    vlogD("Recevied extension friend invite from %s", from);
    vlogD(" ext_data: %.*s", len, (const char *)data);

    write_ack("ext_data %.*s\n", len, data);
}

void *carrier_run_entry(void *arg)
{
    ElaCarrier *w;
    int rc;
    char datadir[PATH_MAX];
    char logfile[PATH_MAX];

    ElaOptions opts = global_config.shared_options;
    opts.log_level = global_config.robot.loglevel;

    opts.persistent_location = datadir;
    sprintf(datadir, "%s/robot", global_config.shared_options.persistent_location);

    if (global_config.log2file) {
        opts.log_file = logfile;
        sprintf(logfile, "%s/robot/robot.log", global_config.shared_options.persistent_location);
    } else {
        opts.log_file = NULL;
    }

    w = ela_new(&opts, &callbacks, &test_context);
    if (!w) {
        write_ack("failed\n");
        vlogE("Carrier new error (0x%x)", ela_get_error());
        return NULL;
    }

    carrier_context.carrier = w;

    rc = extension_init(w, ext_invite_callback, &test_context);
    if (rc != 0) {
        printf("Error: initializing carrier extension error %d.\n", ela_get_error());
        ela_kill(w);
    }

    rc = ela_run(w, 10);
    if (rc != 0) {
        printf("Error: start carrier loop error %d.\n", ela_get_error());
        extension_cleanup(w);
        ela_kill(w);
    }
    carrier_context.carrier = NULL;

    return NULL;
}

int robot_main(int argc, char *argv[])
{
    CarrierContextExtra *extra = carrier_context.extra;
    char *cmd;

    if (start_cmd_listener(global_config.robot.host, global_config.robot.port) < 0)
        return -1;

    pthread_create(&extra->tid, 0, &carrier_run_entry, NULL);

    do {
        cmd = read_cmd();
        do_cmd(&test_context, cmd);
    } while (strcmp(cmd, "kill"));

    pthread_join(extra->tid, NULL);

    stop_cmd_listener();

    vlogI("Carrier exit");

#if defined(_WIN32) || defined(_WIN64)
    // Windows PIPE has no EOF, write a 0xFF indicate end of pipe manually.
    fputc(EOF, stdout);
    fputc(EOF, stderr);
#endif

    fflush(stdout);
    fflush(stderr);

    return 0;
}

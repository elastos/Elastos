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

#include <vlog.h>

#include "ela_carrier.h"
#include "ela_session.h"

#include "test_context.h"
#include "test_helper.h"
#include "config.h"
#include "cmd.h"

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
    vlogD("     presence: %s", info->presence);
}

static void idle_cb(ElaCarrier *w, void *context)
{
    char *cmd = read_cmd();
    if (cmd)
        do_cmd(context, cmd);
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
    TestContext *ctx = (TestContext *)context;
    vlogD("Friend %s's connection status changed -> %s",
          friendid, connection_str(status));

    if (ctx->carrier->fadd_in_progress) {
        ctx->carrier->fadd_in_progress = false;
        // notify api_tests about their connection status change on robot side.
        write_ack("fadd %s\n", status == ElaConnectionStatus_Connected ?
                               "succeeded" : "failed");
    }
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

static void friend_request_cb(ElaCarrier *w, const char *userid,
                const ElaUserInfo *info, const char *hello, void *context)
{
    TestContext *ctx = (TestContext *)context;

    vlogD("Received friend request from user %s", userid);
    print_user_info(info);
    vlogD("  hello: %s", hello);

    if (strcmp(hello, "auto-reply") == 0) {
        int rc;
        ctx->carrier->fadd_in_progress = true;
        rc = ela_accept_friend(w, userid);
        if (rc < 0) {
            vlogE("Accept friend request from %s error (0x%x)",
                            userid, ela_get_error());
            if (ctx->carrier->fadd_in_progress) {
                write_ack("fadd failed\n");
            }
            vlogD("fadd failed");
        } else {
            vlogD("Accept friend request from %s success", userid);
        }

    } else {
        write_ack("hello %s\n", hello);
    }
}

static void friend_added_cb(ElaCarrier *w, const ElaFriendInfo *info,
                            void *context)
{
    vlogI("New friend %s added", info->user_info.userid);
    print_friend_info(info, 0);
}

static void friend_removed_cb(ElaCarrier* w, const char* friendid, void *context)
{
    vlogI("Friend %s is removed", friendid);
    write_ack("fremove succeeded\n");
}

static void friend_message_cb(ElaCarrier *w, const char *from,
                             const void *msg, size_t len, void *context)
{
    vlogD("Received message from %s", from);
    vlogD(" msg: %s", (const char *)msg);

    write_ack("%s\n", msg);
}

static void friend_invite_cb(ElaCarrier *w, const char *from,
                             const void *data, size_t len, void *context)
{
    vlogD("Recevied friend invite from %s", from);
    vlogD(" data: %s", (const char *)data);

    write_ack("data %s\n", data);
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
    .friend_invite   = friend_invite_cb
};

CarrierContext carrier_context = {
    .cbs = &callbacks,
    .carrier = NULL,
    .cond = NULL,
    .extra = NULL
};

static void log_print(const char *format, va_list args)
{
    vprintf(format, args);
}

int robot_main(int argc, char *argv[])
{
    ElaCarrier *w;
    int rc;
    char datadir[PATH_MAX];

    ElaOptions opts = {
        .udp_enabled     = true,
        .bootstraps      = NULL,
        .bootstraps_size = global_config.bootstraps_size,
        .persistent_location = datadir
    };
    int i;

    sprintf(datadir, "%s/robot", global_config.data_location);

    opts.bootstraps = (BootstrapNode *)calloc(1, sizeof(BootstrapNode) * opts.bootstraps_size);
    if (!opts.bootstraps) {
        vlogE("Error: out of memory.");
        return -1;
    }

    for (i = 0 ; i < opts.bootstraps_size; i++) {
        BootstrapNode *b = &opts.bootstraps[i];
        BootstrapNode *node = global_config.bootstraps[i];

        b->ipv4 = node->ipv4;
        b->ipv6 = node->ipv6;
        b->port = node->port;
        b->public_key = node->public_key;
    }

    if (start_cmd_listener(global_config.robot.host, global_config.robot.port) < 0)
        return -1;

    w = ela_new(&opts, &callbacks, &test_context);
    free(opts.bootstraps);

    if (!w) {
        write_ack("failed\n");
        vlogE("Carrier new error (0x%x)", ela_get_error());
        return -1;
    }

    carrier_context.carrier = w;
    rc = ela_run(w, 10);
    carrier_context.carrier = NULL;
    stop_cmd_listener();
    if (rc != 0) {
        vlogE("Carrier run error (0x%x)", ela_get_error());
        ela_kill(w);
        return -1;
    }

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

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "ela_carrier.h"
#include "ela_session.h"
#include "cmd.h"
#include "tests.h"
#include "test_helper.h"

static void print_user_info(const ElaUserInfo* info)
{
    robot_log_debug("       userid: %s\n", info->userid);
    robot_log_debug("         name: %s\n", info->name);
    robot_log_debug("  description: %s\n", info->description);
    robot_log_debug("   has_avatar: %s\n", info->has_avatar ? "true" : "false");
    robot_log_debug("       gender: %s\n", info->gender);
    robot_log_debug("        phone: %s\n", info->phone);
    robot_log_debug("        email: %s\n", info->email);
    robot_log_debug("       region: %s\n", info->region);
}

void print_friend_info(const ElaFriendInfo* info, int order)
{
    if (order > 0)
        robot_log_debug(" friend %d: \n", order);

    print_user_info(&info->user_info);
    robot_log_debug("        label: %s\n", info->label);
    robot_log_debug("     presence: %s\n", info->presence);
}

static void idle_cb(ElaCarrier *w, void *context)
{
    char *cmd = read_cmd();

    if (cmd) {
        robot_log_debug("\n@@@@@@@@ Got command: %s\n", cmd);
        do_cmd(context, cmd);
    }
}

static void connection_status_cb(ElaCarrier *w, ElaConnectionStatus status,
                                 void *context)
{
    robot_log_debug("Robot connection status changed -> %s\n",
                    connection_str(status));
}

static void ready_cb(ElaCarrier *w, void *context)
{
    char address[ELA_MAX_ADDRESS_LEN + 1];
    char robotid[ELA_MAX_ID_LEN + 1];

    ela_get_userid(w, robotid, sizeof(robotid));
    ela_get_address(w, address, sizeof(address));
    

    robot_log_info("Robot is ready\n");
    robot_ack("ready %s %s\n", robotid, address);
}

static
void self_info_cb(ElaCarrier *w, const ElaUserInfo *info, void *context)
{
    robot_log_debug("Received current user information: \n");
    print_user_info(info);
    robot_log_debug("\n");
}

static
bool friend_list_cb(ElaCarrier* w, const ElaFriendInfo *info, void *context)
{
    static bool grouped = false;
    static int idx = 1;

    if (info) {
        if (!grouped) {
            robot_log_debug("Received friend list and listed below:\n");
            grouped = true;
        }
        print_friend_info(info, idx);
        robot_log_debug("\n");
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
    robot_log_debug("Friend %s's connection status changed -> %s\n",
                    friendid, connection_str(status));
}

static void friend_info_cb(ElaCarrier *w, const char *friendid,
                          const ElaFriendInfo *info, void *context)
{
    robot_log_debug("Friend %s's information changed ->\n", friendid);
    print_friend_info(info, 0);
    robot_log_debug("\n");
}

static const char *presence_name[] = {
    "None",
    "Away",
    "Busy"
};

static void friend_presence_cb(ElaCarrier *w, const char *friendid,
                               ElaPresenceStatus status, void *context)
{
    robot_log_info("Friend %s's presence changed -> %s\n", friendid,
                   presence_name[status]);
}

static void friend_request_cb(ElaCarrier *w, const char *userid,
                const ElaUserInfo *info, const char *hello, void *context)
{
    robot_log_debug("Received friend request from user %s\n", userid);
    print_user_info(info);
    robot_log_debug("  hello: %s\n", hello);

    if (strcmp(hello, "auto-reply") == 0) {
        int rc;
        rc = ela_accept_friend(w, userid);
        if (rc < 0) {
            robot_log_error("Accept friend request from %s error (0x%x)\n",
                            userid, ela_get_error());
        } else {
            robot_log_debug("Accept friend request from %s success\n", userid);
        }

    } else {
#if 0
        robot_ack("hello %s\n", hello);
#endif
    }
}

static void friend_added_cb(ElaCarrier *w, const ElaFriendInfo *info,
                            void *context)
{
    robot_log_info("New friend %s added\n", info->user_info.userid);
    print_friend_info(info, 0);
    robot_log_info("\n");
}

static void friend_removed_cb(ElaCarrier* w, const char* friendid, void *context)
{
    robot_log_info("Friend %s is removed\n", friendid);
#if 0
    robot_ack("fremove success\n");
#endif
}

static void friend_message_cb(ElaCarrier *w, const char *from,
                             const char *msg, size_t len, void *context)
{
    robot_log_debug("Received message from %s\n", from);
    robot_log_debug(" msg: %s\n", msg);
    robot_log_debug("\n");

    robot_ack("%s\n", msg);
}

static void friend_invite_cb(ElaCarrier *w, const char *from,
                             const char *data, size_t len, void *context)
{
    robot_log_debug("Recevied friend invite from %s\n", from);
    robot_log_debug(" data: %s\n", data);

    robot_ack("data %s\n", data);
}
static void signal_handler(int signum)
{
    ElaCarrier *w = test_context.carrier->carrier;

    printf("Receive unexpected signal %d, check your code!\n", signum);
    sleep(5);

    if (w)
        ela_kill(w);
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

    ElaOptions opts = {
        .udp_enabled     = true,
        .bootstraps      = NULL,
        .bootstraps_size = global_config.bootstraps_size,
        .persistent_location = global_config.robot.data_location
    };
    int i;

    opts.bootstraps = (BootstrapNode *)calloc(1, sizeof(BootstrapNode) * opts.bootstraps_size);
    if (!opts.bootstraps) {
        test_log_error("Error: out of memory.");
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

    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    //signal(SIGABRT, signal_handler);
    //signal(SIGSEGV, signal_handler);

    char cmd[64];
    wait_robot_ctrl("%64s", cmd);
    if (strcmp(cmd, "start") != 0) {
        return -1;
    }

    //setlinebuf(stdin);
    //setlinebuf(stdout);
    robot_ctrl_nonblock();

    ela_log_init(global_config.robot.loglevel, NULL, log_print);

    w = ela_new(&opts, &callbacks, &test_context);
    free(opts.bootstraps);

    if (!w) {
        robot_ack("failed\n");
        robot_log_error("Carrier new error (0x%x)\n", ela_get_error());
        return -1;
    }

    carrier_context.carrier = w;
    rc = ela_run(w, 10);
    carrier_context.carrier = NULL;
    if (rc != 0) {
        robot_log_error("Carrier run error (0x%x)\n", ela_get_error());
        ela_kill(w);
        return -1;
    }

    robot_log_info("Carrier exit\n");

    return 0;
}

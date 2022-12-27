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
#if __linux__
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <crystal.h>
#include <limits.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>

// Undefine Windows defined MOUSE_MOVED for PDCurses
#undef MOUSE_MOVED
#endif

#include <curses.h>

#ifdef __linux__
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#endif

#include <pthread.h>

#include <ela_carrier.h>
#include <ela_session.h>

#include "carrier_config.h"

#define CONFIG_NAME   "carrier.conf"

static const char *default_config_files[] = {
    "./"CONFIG_NAME,
    "../etc/carrier/"CONFIG_NAME,
#if !defined(_WIN32) && !defined(_WIN64)
    "/usr/local/etc/carrier/"CONFIG_NAME,
    "/etc/carrier/"CONFIG_NAME,
#endif
    NULL
};

#define NUMBER_OF_HISTORY       256

static char default_data_location[PATH_MAX];

static const char *history_filename = ".elashell.history";

static char *cmd_history[NUMBER_OF_HISTORY];
static int cmd_history_last = 0;
static int cmd_history_cursor = 0;
static int cmd_cursor_dir = 1;

WINDOW *output_win_border, *output_win;
WINDOW *log_win_border, *log_win;
WINDOW *cmd_win_border, *cmd_win;

pthread_mutex_t screen_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;

#define OUTPUT_WIN  1
#define LOG_WIN     2
#define CMD_WIN     3

static int OUTPUT_COLS;
static int OUTPUT_LINES = 4;

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

static struct {
    enum {
        INITIATOR,
        RESPONDER
    } role;
    enum {
        IDLE,
        PENDING,
        ONGOING
    } state;
    char peer[ELA_MAX_ID_LEN + 1];
    size_t totalsz;
    size_t sent;
    size_t rcvd;
    struct timeval start;
    struct timeval ul_end;
    struct timeval dl_end;
} bigmsg_benchmark;

static void get_layout(int win, int *w, int *h, int *x, int *y)
{
    if (win == OUTPUT_WIN) {
        if (COLS < 100) {
            *x = 0;
            *y = LINES - (LINES -OUTPUT_LINES) / 2 - OUTPUT_LINES;

            *w = COLS;
            *h = (LINES -OUTPUT_LINES) / 2;
        } else {
            *x = 0;
            *y = 0;

            *w = (COLS - 1) / 2;
            *h = LINES - OUTPUT_LINES;
        }

        OUTPUT_COLS = *w -2;
    } else if (win == LOG_WIN) {
        if (COLS < 100) {
            *x = 0;
            *y = 0;

            *w = COLS;
            *h = LINES - (LINES -OUTPUT_LINES) / 2 - OUTPUT_LINES;
        } else {
            *x = COLS - (COLS / 2);
            *y = 0;

            *w = (COLS - 1) / 2;
            *h = LINES - OUTPUT_LINES;
        }
    } else if (win == CMD_WIN) {
        if (COLS < 100) {
            *x = 0;
            *y = LINES - OUTPUT_LINES;

            *w = COLS;
            *h = OUTPUT_LINES;
        } else {
            *x = 0;
            *y = LINES - OUTPUT_LINES;

            *w = COLS;
            *h = OUTPUT_LINES;
        }
    }
}

#ifdef HAVE_SIGACTION
static void handle_winch(int sig)
{
    int w, h, x, y;

    endwin();

    if (LINES < 20 || COLS < 80) {
        printf("Terminal size too small!\n");
        exit(-1);
    }

    refresh();
    clear();

    wresize(stdscr, LINES, COLS);

    get_layout(OUTPUT_WIN, &w, &h, &x, &y);

    wresize(output_win_border, h, w);
    mvwin(output_win_border, y, x);
    box(output_win_border, 0, 0);
    mvwprintw(output_win_border, 0, 4, "Output");

    wresize(output_win, h-2, w-2);
    mvwin(output_win, y+1, x+1);

    get_layout(LOG_WIN, &w, &h, &x, &y);

    wresize(log_win_border, h, w);
    mvwin(log_win_border, y, x);
    box(log_win_border, 0, 0);
    mvwprintw(log_win_border, 0, 4, "Log");

    wresize(log_win, h-2, w-2);
    mvwin(log_win, y+1,  x+1);

    get_layout(CMD_WIN, &w, &h, &x, &y);

    wresize(cmd_win_border, h, w);
    mvwin(cmd_win_border, y, x);
    box(cmd_win_border, 0, 0);
    mvwprintw(cmd_win_border, 0, 4, "Command");

    wresize(cmd_win, h-2, w-2);
    mvwin(cmd_win,  y+1,  x+1);

    clear();
    refresh();

    wrefresh(output_win_border);
    wrefresh(output_win);

    wrefresh(log_win_border);
    wrefresh(log_win);

    wrefresh(cmd_win_border);
    wrefresh(cmd_win);
}
#endif

static void init_screen(void)
{
    int w, h, x, y;

    initscr();

    if (LINES < 20 || COLS < 80) {
        printf("Terminal size too small!\n");
        endwin();
        exit(-1);
    }

    noecho();
    nodelay(stdscr, TRUE);
    refresh();

    get_layout(OUTPUT_WIN, &w, &h, &x, &y);

    output_win_border = newwin(h, w, y, x);
    box(output_win_border, 0, 0);
    mvwprintw(output_win_border, 0, 4, "Output");
    wrefresh(output_win_border);

    output_win = newwin(h-2, w-2, y+1, x+1);
    scrollok(output_win, TRUE);
    wrefresh(output_win);

    get_layout(LOG_WIN, &w, &h, &x, &y);

    log_win_border = newwin(h, w, y, x);
    box(log_win_border, 0, 0);
    mvwprintw(log_win_border, 0, 4, "Log");
    wrefresh(log_win_border);

    log_win = newwin(h-2, w-2, y+1,  x+1);
    scrollok(log_win, TRUE);
    wrefresh(log_win);

    get_layout(CMD_WIN, &w, &h, &x, &y);

    cmd_win_border = newwin(h, w, y, x);
    box(cmd_win_border, 0, 0);
    mvwprintw(cmd_win_border, 0, 4, "Command");
    wrefresh(cmd_win_border);

    cmd_win = newwin(h-2, w-2, y+1,  x+1);
    scrollok(cmd_win, true);
    waddstr(cmd_win, "# ");
    wrefresh(cmd_win);

#ifdef HAVE_SIGACTION
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);
#endif
}

static void cleanup_screen(void)
{
    endwin();

    delwin(output_win_border);
    delwin(output_win);

    delwin(log_win_border);
    delwin(log_win);

    delwin(cmd_win_border);
    delwin(cmd_win);
}

static void history_load(void)
{
    int i = 0;
    char filename[PATH_MAX];
    FILE *fp;
    char line[1024];
    char *p;

    sprintf(filename, "%s/%s", default_data_location, history_filename);

    fp = fopen(filename, "r");
    if (!fp)
        return;

    while (fgets(line, sizeof(line), fp)) {
        // Trim trailing spaces;
        for (p = line + strlen(line) - 1; p >= line && isspace(*p); p--);
        *(++p) = 0;

        // Trim leading spaces;
        for (p = line; *p && isspace(*p); p++);

        if (strlen(p) == 0)
            continue;

        cmd_history[i] = strdup(p);

        i = (i + 1) % NUMBER_OF_HISTORY;
    }

    cmd_history_last = i;
    cmd_history_cursor = cmd_history_last;

    fclose(fp);
}

static void history_save(void)
{
    int i = 0;
    char filename[PATH_MAX];
    FILE *fp;

    sprintf(filename, "%s/%s", default_data_location, history_filename);

    fp = fopen(filename, "w");
    if (!fp)
        return;

    i = cmd_history_last;
    do {
        if (cmd_history[i]) {
            fprintf(fp, "%s\n", cmd_history[i]);
            free(cmd_history[i]);
            cmd_history[i] = NULL;
        }

        i = (i + 1) % NUMBER_OF_HISTORY;
    } while (i != cmd_history_last);

    fclose(fp);
}

static void history_add_cmd(const char *cmd)
{
    if (cmd_history[cmd_history_last])
        free(cmd_history[cmd_history_last]);

    cmd_history[cmd_history_last] = strdup(cmd);

    cmd_history_last = (cmd_history_last + 1) % NUMBER_OF_HISTORY;
    cmd_history_cursor = cmd_history_last;
    cmd_cursor_dir = 1;
}

static const char *history_prev(void)
{
    int n;
    const char *cmd = NULL;

    if (cmd_cursor_dir == -1 &&
        (cmd_history_cursor == cmd_history_last ||
         cmd_history[cmd_history_cursor] == NULL))
        return NULL;

    n = (cmd_history_cursor - 1 + NUMBER_OF_HISTORY) % NUMBER_OF_HISTORY;
    cmd_history_cursor = n;

    if (cmd_history[n])
        cmd = cmd_history[n];

    cmd_cursor_dir = -1;

    return cmd;
}

static const char *history_next(void)
{
    int n;
    const char *cmd = NULL;

    if (cmd_cursor_dir == 1 && cmd_history_cursor == cmd_history_last)
        return NULL;

    n = (cmd_history_cursor + 1) % NUMBER_OF_HISTORY;
    cmd_history_cursor = n;

    if (cmd_history_cursor != cmd_history_last)
        cmd = cmd_history[n];

    cmd_cursor_dir = 1;

    return cmd;
}

static void log_print(const char *format, va_list args)
{
    pthread_mutex_lock(&screen_lock);
    vwprintw(log_win, format, args);
    wrefresh(log_win);
    wrefresh(cmd_win);
    pthread_mutex_unlock(&screen_lock);
}

static void output(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    pthread_mutex_lock(&screen_lock);
    vwprintw(output_win, format, args);
    wrefresh(output_win);
    wrefresh(cmd_win);
    pthread_mutex_unlock(&screen_lock);

    va_end(args);
}

static void clear_screen(ElaCarrier *w, int argc, char *argv[])
{
    if (argc == 1) {
        pthread_mutex_lock(&screen_lock);
        wclear(output_win);
        wrefresh(output_win);
        wclear(log_win);
        wrefresh(log_win);
        wrefresh(cmd_win);
        pthread_mutex_unlock(&screen_lock);
    } else if (argc == 2) {
        WINDOW *w;
        if (strcmp(argv[1], "log") == 0)
            w = log_win;
        else if (strcmp(argv[1], "out") == 0)
            w = output_win;
        else {
            output("Invalid command syntax.\n");
            return;
        }

        pthread_mutex_lock(&screen_lock);
        wclear(w);
        wrefresh(w);
        wrefresh(cmd_win);
        pthread_mutex_unlock(&screen_lock);
    } else {
        output("Invalid command syntax.\n");
        return;
    }
}

static void get_address(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    char addr[ELA_MAX_ADDRESS_LEN+1] = {0};
    ela_get_address(w, addr, sizeof(addr));
    output("Address: %s\n", addr);
}

static void get_nodeid(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    char id[ELA_MAX_ID_LEN+1] = {0};
    ela_get_nodeid(w, id, sizeof(id));
    output("Node ID: %s\n", id);
}

static void get_userid(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    char id[ELA_MAX_ID_LEN+1] = {0};
    ela_get_userid(w, id, sizeof(id));
    output("User ID: %s\n", id);
}

static void display_user_info(const ElaUserInfo *info)
{
    output("           ID: %s\n", info->userid);
    output("         Name: %s\n", info->name);
    output("  Description: %s\n", info->description);
    output("       Gender: %s\n", info->gender);
    output("        Phone: %s\n", info->phone);
    output("        Email: %s\n", info->email);
    output("       Region: %s\n", info->region);
}

static void self_info(ElaCarrier *w, int argc, char *argv[])
{
    ElaUserInfo info;
    int rc;

    rc = ela_get_self_info(w, &info);
    if (rc != 0) {
        output("Get self information failed(0x%x).\n", ela_get_error());
        return;
    }

    if (argc == 1) {
        output("Self information:\n");
        display_user_info(&info);
    } else if (argc == 3 || argc == 4) {
        const char *value = "";
        if (strcmp(argv[1], "set") != 0) {
            output("Unknown sub command: %s\n", argv[1]);
            return;
        }

        if (argc == 4)
            value = argv[3];

        if (strcmp(argv[2], "name") == 0) {
            strncpy(info.name, value, sizeof(info.name));
            info.name[sizeof(info.name)-1] = 0;
        } else if (strcmp(argv[2], "description") == 0) {
            strncpy(info.description, value, sizeof(info.description));
            info.description[sizeof(info.description)-1] = 0;
        } else if (strcmp(argv[2], "gender") == 0) {
            strncpy(info.gender, value, sizeof(info.gender));
            info.gender[sizeof(info.gender)-1] = 0;
        } else if (strcmp(argv[2], "phone") == 0) {
            strncpy(info.phone, value, sizeof(info.phone));
            info.phone[sizeof(info.phone)-1] = 0;
        } else if (strcmp(argv[2], "email") == 0) {
            strncpy(info.email, value, sizeof(info.email));
            info.email[sizeof(info.email)-1] = 0;
        } else if (strcmp(argv[2], "region") == 0) {
            strncpy(info.region, value, sizeof(info.region));
            info.region[sizeof(info.region)-1] = 0;
        } else {
            output("Invalid attribute name: %s\n", argv[2]);
            return;
        }

        ela_set_self_info(w, &info);
    } else {
        output("Invalid command syntax.\n");
    }
}

static void self_nospam(ElaCarrier *w, int argc, char *argv[])
{
    uint32_t nospam;

    if (argc == 1) {
        ela_get_self_nospam(w, &nospam);
        output("Self nospam: %lu.\n", nospam);
        return;
    } else if (argc == 2) {
        nospam = (uint32_t)atoi(argv[1]);
        if (nospam == 0) {
            output("Invalid nospam value to set.");
            return;
        }

        ela_set_self_nospam(w, nospam);
        output("User's nospam changed to be %lu.\n", nospam);
    } else {
        output("Invalid command syntax.\n");
    }
}

const char *presence_name[] = {
    "none",    // None;
    "away",    // Away;
    "busy",    // Busy;
};

static void self_presence(ElaCarrier *w, int argc, char *argv[])
{
    ElaPresenceStatus presence;
    int rc;

    if (argc == 1) {
        ela_get_self_presence(w, &presence);
        output("Self presence: %s\n", presence_name[presence]);
        return;
    } else if (argc == 2) {
        if (strcmp(argv[1], "none") == 0)
            presence = ElaPresenceStatus_None;
        else if (strcmp(argv[1], "away") == 0)
            presence = ElaPresenceStatus_Away;
        else if (strcmp(argv[1], "busy") == 0)
            presence = ElaPresenceStatus_Busy;
        else {
            output("Invalid command syntax.\n");
            return;
        }

        rc = ela_set_self_presence(w, presence);
        if (rc < 0)
            output("Set user's presence failed (0x%x)\n", ela_get_error());
        else
            output("User's presence changed to be %s.\n", argv[1]);
    } else {
        output("Invalid command syntax.\n");
    }
}

static void friend_add(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_add_friend(w, argv[1], argv[2]);
    if (rc == 0)
        output("Request to add a new friend succeess.\n");
    else
        output("Request to add a new friend failed(0x%x).\n",
                ela_get_error());
}

static void friend_accept(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_accept_friend(w, argv[1]);
    if (rc == 0)
        output("Accept friend request success.\n");
    else
        output("Accept friend request failed(0x%x).\n", ela_get_error());
}

static void friend_remove(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_remove_friend(w, argv[1]);
    if (rc == 0)
        output("Remove friend %s success.\n", argv[1]);
    else
        output("Remove friend %s failed (0x%x).\n", argv[1], ela_get_error());
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
        output("Friends list from carrier network:\n");
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
static bool get_friends_callback(const ElaFriendInfo *friend_info, void *context)
{
    static int count;

    if (first_friends_item) {
        count = 0;
        output("Friends list:\n");
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

static void list_friends(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    ela_get_friends(w, get_friends_callback, NULL);
}

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

static void show_friend(ElaCarrier *w, int argc, char *argv[])
{
    int rc;
    ElaFriendInfo fi;

    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_get_friend_info(w, argv[1], &fi);
    if (rc < 0) {
        output("Get friend information failed(0x%x).\n", ela_get_error());
        return;
    }

    output("Friend %s information:\n", argv[1]);
    display_friend_info(&fi);
}

static void label_friend(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_set_friend_label(w, argv[1], argv[2]);
    if (rc == 0)
        output("Update friend label success.\n");
    else
        output("Update friend label failed(0x%x).\n", ela_get_error());
}

static void friend_added_callback(ElaCarrier *w, const ElaFriendInfo *info,
                                  void *context)
{
    output("New friend added. The friend information:\n");
    display_friend_info(info);
}

static void friend_removed_callback(ElaCarrier *w, const char *friendid,
                                    void *context)
{
    output("Friend %s removed!\n", friendid);
}

static void send_message(ElaCarrier *w, int argc, char *argv[])
{
    bool is_offline;
    int rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_send_friend_message(w, argv[1], argv[2], strlen(argv[2]) + 1,
                                 &is_offline);
    if (rc == 0)
        output("Send %s message success.\n", is_offline ? "offline" : "online");
    else
        output("Send message failed(0x%x).\n", ela_get_error());
}

static void send_bulk_message(ElaCarrier *w, int argc, char *argv[])
{
    int total_count;
    int failed_count;
    size_t msglen;
    int i;
    int rc;

    if (argc != 4) {
        output("Invalid command syntax.\n");
        return;
    }

    total_count = atoi(argv[2]);
    if (total_count <= 0) {
        output("Count is invalid.\n");
        return;
    }

    msglen = strlen(argv[3]);
    if (msglen >= ELA_MAX_APP_MESSAGE_LEN - 16) {
        output("Message is too long.\n");
        return;
    }

    output("Sending");
    failed_count = 0;
    for (i = 0; i < total_count; i++) {
        char msg[ELA_MAX_APP_MESSAGE_LEN + 1] = {0};
        char index[16] = {0};

        sprintf(index, "#%d", i + 1);
        strcpy(msg, argv[3]);
        strcat(msg, index);

        rc = ela_send_friend_message(w, argv[1], msg, strlen(msg) + 1, NULL);
        if (rc < 0) {
            output("x(0x%x)", ela_get_error());
            failed_count++;
        } else {
            output(".");
        }
    }

    output("\nSend bulk messages finished\n");
    output("  totoal: %d\n", total_count);
    output(" success: %d\n", total_count - failed_count);
    output("  failed: %d\n", failed_count);
}

static void receipt_message_callback(int64_t msgid,  ElaReceiptState state,
                                     void *context)
{
    const char* state_str;
    int errcode = 0;

    switch (state) {
        case ElaReceipt_ByFriend:
            state_str = "Friend receipt";
            break;
        case ElaReceipt_Offline:
            state_str = "Offline";
            break;
        case ElaReceipt_Error:
            state_str = "Error";
            errcode = ela_get_error();
            break;
        default:
            state_str = "Unknown";
            break;
    }

    output("Messages receipted. msgid:0x%llx, state:%s, ecode:%x\n",
           msgid, state_str, errcode);
}

static void send_receipt_message(ElaCarrier *w, int argc, char *argv[])
{
    int64_t msgid;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    msgid = ela_send_message_with_receipt(w, argv[1], argv[2], strlen(argv[2]) + 1,
                                          receipt_message_callback, NULL);
    if (msgid >= 0)
        output("Sending receipt message. msgid:0x%llx\n", msgid);
    else
        output("Send message failed(0x%x).\n", ela_get_error());
}

static void send_receipt_bulkmessage(ElaCarrier *w, int argc, char *argv[])
{
    int rc;
    int datalen = 2048;
    char *data;
    int idx;

    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    data = (char*)calloc(1, datalen);
    for(idx = 0; idx < datalen; idx++) {
        data[idx] = '0' + (idx % 8);
    }
    memcpy(data + datalen - 5, "end", 4);

    int64_t msgid = ela_send_message_with_receipt(w, argv[1], data, strlen(data) + 1,
                                                  receipt_message_callback, NULL);
    free(data);
    if (msgid >= 0)
        output("Sending receipt message. msgid:0x%llx\n", msgid);
    else
        output("Send message failed(0x%x).\n", ela_get_error());
}

static void bigmsg_benchmark_initialize(ElaCarrier *w, int argc, char *argv[])
{
    size_t totalsz;
    char ctlsig[256];
    int rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    if (!ela_id_is_valid(argv[1])) {
        output("User ID is invalid.\n");
        return;
    }

    totalsz = atoi(argv[2]);
    if (totalsz <= 0 || totalsz > 10240) {
        output("Count is invalid.\n");
        return;
    }

    if (bigmsg_benchmark.state != IDLE) {
        output("Invalid state.\n");
        return;
    }

    memset(&bigmsg_benchmark, 0, sizeof(bigmsg_benchmark));
    bigmsg_benchmark.role = INITIATOR;
    bigmsg_benchmark.state = PENDING;
    strcpy(bigmsg_benchmark.peer, argv[1]);
    bigmsg_benchmark.totalsz = totalsz << 20;

    rc = sprintf(ctlsig, "bigmsgbenchmark request %s", argv[2]);
    rc = ela_send_friend_message(w, argv[1], ctlsig, rc, NULL);
    if (rc < 0) {
        output("Send bigmessage benchmark request error.\n");
        bigmsg_benchmark.state = IDLE;
    }

    output("Bigmessage benchmark request sent.\n");
}

static void *bigmsg_benchmark_write_thread(void *arg)
{
    ElaCarrier *w = (ElaCarrier *)arg;
    char *buf = calloc(1, ELA_MAX_APP_BULKMSG_LEN);

    while (bigmsg_benchmark.state == ONGOING && bigmsg_benchmark.sent < bigmsg_benchmark.totalsz) {
        size_t len = bigmsg_benchmark.totalsz - bigmsg_benchmark.sent;
        int rc;

        if (len > ELA_MAX_APP_BULKMSG_LEN)
            len = ELA_MAX_APP_BULKMSG_LEN;

        rc = ela_send_friend_message(w, bigmsg_benchmark.peer, buf, len, NULL);
        if (rc < 0) {
            usleep(100000);
            continue;
        }

        bigmsg_benchmark.sent += len;
    }

    free(buf);
    gettimeofday(&bigmsg_benchmark.ul_end, NULL);
    return NULL;
}

static void calculate_data_rate(size_t datasz, const struct timeval *interval, char *result)
{
    double interval_sec;
    double rate;

    interval_sec = interval->tv_sec + interval->tv_usec * (10E-6);

    rate = datasz / interval_sec;
    if (((size_t)rate) >> 30)
        sprintf(result, "%6.1lfGB/s", rate / (1U << 30));
    else if (((size_t)rate) >> 20)
        sprintf(result, "%6.1lfMB/s", rate / (1U << 20));
    else if (((size_t)rate) >> 10)
        sprintf(result, "%6.1lfKB/s", rate / (1U << 10));
    else
        sprintf(result, "%6.1lf B/s", rate);
}

static void calculate_progress(size_t cur, size_t total, char *result)
{
    sprintf(result, "%5.1lf%%", (double)cur / total * 100);
}

static void *monitor_bigmsg_benchmark_progress(void *arg)
{
    struct timeval update_rate = {
        .tv_sec = 1,
        .tv_usec = 0
    };
    struct timeval dl_time_elapsed;
    struct timeval ul_time_elapsed;
    char ul_rate[128];
    char dl_rate[128];
    char ul_progress[128];
    char dl_progress[128];
    size_t last_sent = 0;
    size_t last_rcvd = 0;
    size_t cur_sent;
    size_t cur_rcvd;

    output("Upload:    0.0B/s[  0.0%%], Download:    0.0B/s[  0.0%%]");

    do {
        sleep(1);

        cur_sent = bigmsg_benchmark.sent;
        cur_rcvd = bigmsg_benchmark.rcvd;
        calculate_data_rate(cur_sent - last_sent, &update_rate, ul_rate);
        calculate_data_rate(cur_rcvd - last_rcvd, &update_rate, dl_rate);
        calculate_progress(cur_sent, bigmsg_benchmark.totalsz, ul_progress);
        calculate_progress(cur_rcvd, bigmsg_benchmark.totalsz, dl_progress);

        output("\rUpload: %s[%s], Download: %s[%s]", ul_rate, ul_progress, dl_rate, dl_progress);
        last_sent = cur_sent;
        last_rcvd = cur_rcvd;
    } while (bigmsg_benchmark.state == ONGOING &&
             (cur_sent != bigmsg_benchmark.totalsz ||
              cur_rcvd != bigmsg_benchmark.totalsz));

    if (bigmsg_benchmark.state != ONGOING) {
        output("\nBigmessage benchmark aborted.\n");
        return NULL;
    }

    timersub(&bigmsg_benchmark.ul_end, &bigmsg_benchmark.start, &ul_time_elapsed);
    timersub(&bigmsg_benchmark.dl_end, &bigmsg_benchmark.start, &dl_time_elapsed);
    calculate_data_rate(bigmsg_benchmark.totalsz, &ul_time_elapsed, ul_rate);
    calculate_data_rate(bigmsg_benchmark.totalsz, &dl_time_elapsed, dl_rate);
    output("\nBenchmark Summary:\n"
           "  Total Transfer Size: %zuMb\n"
           "  Average Upload Rate: %s\n"
           "  Average Download Rate: %s\n",
           bigmsg_benchmark.totalsz >> 20, ul_rate, dl_rate);
    bigmsg_benchmark.state = IDLE;

    return NULL;
}

static void bigmsg_benchmark_accept(ElaCarrier *w, int argc, char *argv[])
{
    const char *ctlsig = "bigmsgbenchmark accept";
    pthread_attr_t attr;
    pthread_t th;
    int rc;

    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    if (bigmsg_benchmark.state != PENDING || bigmsg_benchmark.role != RESPONDER) {
        output("Invalid state.\n");
        return;
    }

    rc = ela_send_friend_message(w, bigmsg_benchmark.peer, ctlsig, strlen(ctlsig), NULL);
    if (rc < 0) {
        output("Failed to send bigmsgbenchmark accept signal.\n");
        return;
    }

    output("Send bigmsgbenchmark accept signal, start benchmark.\n");

    bigmsg_benchmark.state = ONGOING;
    gettimeofday(&bigmsg_benchmark.start, NULL);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&th, &attr, bigmsg_benchmark_write_thread, w);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&th, &attr, monitor_bigmsg_benchmark_progress, NULL);
    pthread_attr_destroy(&attr);
}

static int bigmsg_benchmark_send_reject(ElaCarrier *w, const char *to)
{
    const char *ctlsig = "bigmsgbenchmark reject";

    return ela_send_friend_message(w, to, ctlsig, strlen(ctlsig), NULL);
}

static void bigmsg_benchmark_reject(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    if (bigmsg_benchmark.state != PENDING || bigmsg_benchmark.role != RESPONDER) {
        output("Invalid state.\n");
        return;
    }

    rc = bigmsg_benchmark_send_reject(w, bigmsg_benchmark.peer);
    if (rc < 0) {
        output("Send bigmsgbenchmark reject error.\n");
        return;
    }

    output("bigmessage benchmark is rejected.\n");

    bigmsg_benchmark.state = IDLE;
}

static void invite_response_callback(ElaCarrier *w, const char *friendid,
                            const char *bundle, int status, const char *reason,
                            const void *data, size_t len, void *context)
{
    output("Got invite response from %s. ", friendid);
    if (status == 0) {
        output("message within response: %.*s ", (int)len, (const char *)data);
    } else {
        output("refused: %s ", reason);
    }
    output("and bundle: %s\n",  bundle ? bundle : "N/A");
}

static void invite(ElaCarrier *w, int argc, char *argv[])
{
    const char *bundle = NULL;
    int rc;

    if (argc != 3 && argc != 4) {
        output("Invalid command syntax.\n");
        return;
    }

    if (argc == 4)
        bundle = argv[3];

    rc = ela_invite_friend(w, argv[1], bundle, argv[2], strlen(argv[2]),
                               invite_response_callback, NULL);
    if (rc == 0)
        output("Send invite request success.\n");
    else
        output("Send invite request failed(0x%x).\n", ela_get_error());
}

static void reply_invite(ElaCarrier *w, int argc, char *argv[])
{
    int rc;
    int status = 0;
    const char *bundle = NULL;
    const char *reason = NULL;
    const char *msg = NULL;
    size_t msg_len = 0;

    if (argc != 4 && argc != 5) {
        output("Invalid command syntax.\n");
        return;
    }

    if (strcmp(argv[2], "confirm") == 0) {
        msg = argv[3];
        msg_len = strlen(argv[3]);
    } else if (strcmp(argv[2], "refuse") == 0) {
        status = -1; // TODO: fix to correct status code.
        reason = argv[3];
    } else {
        output("Unknown sub command: %s\n", argv[2]);
        return;
    }

    if (argc == 5)
        bundle = argv[4];

    rc = ela_reply_friend_invite(w, argv[1], bundle, status, reason, msg, msg_len);
    if (rc == 0)
        output("Send invite reply to inviter success.\n");
    else
        output("Send invite reply to inviter failed(0x%x).\n", ela_get_error());
}

static void kill_carrier(ElaCarrier *w, int argc, char *argv[])
{
    ela_kill(w);
}

static void session_request_callback(ElaCarrier *w, const char *from,
            const char *bundle, const char *sdp, size_t len, void *context)
{
    strncpy(session_ctx.remote_sdp, sdp, len);
    session_ctx.remote_sdp[len] = 0;
    session_ctx.sdp_len = len;

    assert(!bundle);

    output("Session request from[%s]\nSDP:\n%s.\n", from, session_ctx.remote_sdp);
    output("Reply use following commands:\n");
    output("  sreply refuse [reason]\n");
    output("OR:\n");
    output("  1. snew %s\n", from);
    output("  2. sadd [plain] [reliable] [multiplexing] [portforwarding]\n");
    output("  3. sreply ok\n");
}

static void session_request_complete_callback(ElaSession *ws, const char *bundle, int status,
                const char *reason, const char *sdp, size_t len, void *context)
{
    int rc;

    assert(!bundle);

    output("Session complete, status: %d, reason: %s\n", status,
           reason ? reason : "N/A");

    if (status != 0)
        return;

    strncpy(session_ctx.remote_sdp, sdp, len);
    session_ctx.remote_sdp[len] = 0;
    session_ctx.sdp_len = len;

    rc = ela_session_start(session_ctx.ws, session_ctx.remote_sdp,
                               session_ctx.sdp_len);

    output("Session start %s.\n", rc == 0 ? "success" : "failed");
}

static void stream_bulk_receive(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    if (strcmp(argv[1], "start") == 0) {
        session_ctx.bulk_mode = 1;
        session_ctx.bytes = 0;
        session_ctx.packets = 0;

        output("Ready to receive bulk data");
    } else if (strcmp(argv[1], "end") == 0) {
        struct timeval start = session_ctx.first_stamp;
        struct timeval end = session_ctx.last_stamp;

        int duration = (int)((end.tv_sec - start.tv_sec) * 1000000 +
                             (end.tv_usec - start.tv_usec)) / 1000;
        duration = (duration == 0)  ? 1 : duration;
        float speed = (float)((session_ctx.bytes / duration) * 1000) / 1024;

        output("\nFinish! Total %" PRIu64 " bytes in %d.%03d seconds. %.2f KB/s\n",
               session_ctx.bytes,
               (int)(duration / 1000), (int)(duration % 1000), speed);

        session_ctx.bulk_mode = 0;
        session_ctx.bytes = 0;
        session_ctx.packets = 0;
    } else  {
        output("Invalid command syntax.\n");
        return;
    }
}

static void stream_on_data(ElaSession *ws, int stream, const void *data,
                           size_t len, void *context)
{
    if (session_ctx.bulk_mode) {
        if (session_ctx.packets % 1000 == 0)
            output(".");

        if (session_ctx.packets)
            gettimeofday(&session_ctx.last_stamp, NULL);
        else
            gettimeofday(&session_ctx.first_stamp, NULL);

        session_ctx.bytes += len;
        session_ctx.packets++;
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
}

bool on_channel_open(ElaSession *ws, int stream, int channel,
                     const char *cookie, void *context)
{
    output("Stream request open new channel %d.\n", channel);
    return true;
}

void on_channel_opened(ElaSession *ws, int stream, int channel, void *context)
{
    output("Channel %d:%d opened.\n", stream, channel);
}

void on_channel_close(ElaSession *ws, int stream, int channel,
                      CloseReason reason, void *context)
{
    output("Channel %d:%d closed.\n", stream, channel);
}

bool on_channel_data(ElaSession *ws, int stream, int channel,
                     const void *data, size_t len, void *context)
{
    output("Channel %d:%d received data [%s]\n", stream, channel, data);
    return true;
}

void on_channel_pending(ElaSession *ws, int stream, int channel, void *context)
{
    output("Channel %d:%d is pending.\n", stream, channel);
}

void on_channel_resume(ElaSession *ws, int stream, int channel, void *context)
{
    output("Channel %d:%d resumed.\n", stream, channel);
}

static void group_new(ElaCarrier *w, int argc, char *argv[])
{
    int rc;
    char groupid[ELA_MAX_ID_LEN + 1];

    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_new_group(w, groupid, sizeof(groupid));
    if (rc < 0) {
        output("Create group failed.\n");
    } else {
        output("Create group[%s] successfully.\n", groupid);
    }
}

static void group_leave(ElaCarrier *w, int argc, char **argv)
{
    int rc;

    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_leave_group(w, argv[1]);
    if (rc < 0) {
        output("Exit group[%s] failed.\n", argv[1]);
    } else {
        output("Exit group[%s] successfully.\n", argv[1]);
    }
}

static void group_invite(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_group_invite(w, argv[1], argv[2]);
    if (rc < 0) {
        output("Invite friend[%s] into group[%s] failed.\n", argv[2], argv[1]);
    } else {
        output("Invite friend[%s] into group[%s] successfully.\n", argv[2], argv[1]);
    }
}

static void group_join(ElaCarrier *w, int argc, char *argv[])
{
    char groupid[ELA_MAX_ID_LEN + 1];
    size_t cookie_len;
    uint8_t *cookie;
    size_t i;
    int rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    cookie_len = strlen(argv[2]) >> 1;
    cookie = (uint8_t *)alloca(cookie_len);

    for (i = 0; i < cookie_len; i++) {
        sscanf(argv[2] + (i << 1), "%2hhX", cookie + i);
    }

    rc = ela_group_join(w, argv[1], cookie, cookie_len, groupid, sizeof(groupid));
    if (rc < 0) {
        output("Join in group failed.\n");
    } else {
        output("Join in group[%s] successfully.\n", groupid);
    }
}

static void group_send_message(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_group_send_message(w, argv[1], argv[2], strlen(argv[2]));
    if (rc < 0) {
        output("Send group[%s] message failed.\n", argv[1]);
    } else {
        output("Send group[%s] message successfully.\n", argv[1]);
    }
}

static void group_set_title(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 2 && argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    if (argc == 2) {
        char title[ELA_MAX_GROUP_TITLE_LEN + 1];

        rc = ela_group_get_title(w, argv[1], title, sizeof(title));
        if (rc < 0) {
            output("Get group[%s] title failed.\n", argv[1]);
        } else {
            output("Group[%s] title is [%s].\n", argv[1], title);
        }
    } else {
        rc = ela_group_set_title(w, argv[1], argv[2]);
        if (rc < 0) {
            output("Set group[%s] title failed.\n", argv[1]);
        } else {
            output("Set group title successfully.\n", argv[1]);
        }
    }
}

static bool print_group_peer_info(const ElaGroupPeer *peer, void *context)
{
    int *peer_number = (int *)context;

    if (!peer) {
        return false;
    }

    output("%d. %s[%s]\n", (*peer_number)++, peer->name, peer->userid);
    return true;
}

static void group_list_peers(ElaCarrier *w, int argc, char *argv[])
{
    int peer_number = 0;
    int rc;

    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    output("Group[%s] peers:\n", argv[1]);
    rc = ela_group_get_peers(w, argv[1], print_group_peer_info,
                             (void *)&peer_number);
    if (rc < 0) {
        output("List group peers failed.\n");
    }
}

static bool print_group_id(const char *groupid, void *context)
{
    int *group_number = (int *)context;

    if (!groupid) {
        return false;
    }

    output("%d. %s\n", (*group_number)++, groupid);
    return true;
}

static void group_list(ElaCarrier *w, int argc, char *argv[])
{
    int rc;
    int group_number = 0;

    output("Group IDs:\n");
    rc = ela_get_groups(w, print_group_id, (void *)&group_number);
    if (rc < 0) {
        output("List groups failed.\n");
    }
}

static void session_init(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_session_init(w);
    if (rc < 0) {
        output("Session initialized failed.\n");
    }
    else {
        ela_session_set_callback(w, NULL, session_request_callback, NULL);
        output("Session initialized successfully.\n");
    }
}

static void session_cleanup(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    ela_session_cleanup(w);
    output("Session cleaned up.\n");
}

static void session_new(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    session_ctx.ws = ela_session_new(w, argv[1]);
    if (!session_ctx.ws) {
        output("Create session failed.\n");
    } else {
        output("Create session successfully.\n");
    }
    session_ctx.unchanged_streams = 0;
}

static void session_close(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    if (session_ctx.ws) {
        ela_session_close(session_ctx.ws);
        session_ctx.ws = NULL;
        session_ctx.remote_sdp[0] = 0;
        session_ctx.sdp_len = 0;
        output("Session closed.\n");
    } else {
        output("No session available.\n");
    }
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
        output("Invalid command syntax.\n");
        return;
    } else if (argc > 1) {
        int i;

        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i], "reliable") == 0) {
                options |= ELA_STREAM_RELIABLE;
            } else if (strcmp(argv[i], "plain") == 0) {
                options |= ELA_STREAM_PLAIN;
            } else if (strcmp(argv[i], "multiplexing") == 0) {
                options |= ELA_STREAM_MULTIPLEXING;
            } else if (strcmp(argv[i], "portforwarding") == 0) {
                options |= ELA_STREAM_PORT_FORWARDING;
            } else {
                output("Invalid command syntax.\n");
                return;
            }
        }
    }

    if ((options & ELA_STREAM_MULTIPLEXING) || (options & ELA_STREAM_PORT_FORWARDING)) {
        callbacks.channel_open = on_channel_open;
        callbacks.channel_opened = on_channel_opened;
        callbacks.channel_data = on_channel_data;
        callbacks.channel_pending = on_channel_pending;
        callbacks.channel_resume = on_channel_resume;
        callbacks.channel_close = on_channel_close;
    }

    rc = ela_session_add_stream(session_ctx.ws, ElaStreamType_text,
                                options, &callbacks, NULL);
    if (rc < 0) {
        output("Add stream failed.\n");
    }
    else {
        session_ctx.unchanged_streams++;
        output("Add stream successfully and stream id %d.\n", rc);
    }
}

static void stream_remove(ElaCarrier *w, int argc, char *argv[])
{
    int rc;
    int stream;

    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    stream = atoi(argv[1]);
    rc = ela_session_remove_stream(session_ctx.ws, stream);
    if (rc < 0) {
        output("Remove stream %d failed.\n", stream);
    }
    else {
        output("Remove stream %d success.\n", stream);
    }
}

static void session_request(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 1) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_session_request(session_ctx.ws, NULL,
                             session_request_complete_callback, NULL);
    if (rc < 0) {
        output("session request failed.\n");
    }
    else {
        output("session request successfully.\n");
    }
}

static void session_reply_request(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if ((argc != 2) && (argc != 3)) {
        output("Invalid command syntax.\n");
        return;
    }

    if ((strcmp(argv[1], "ok") == 0) && (argc == 2)) {
        rc = ela_session_reply_request(session_ctx.ws, NULL, 0, NULL);
        if (rc < 0) {
            output("response invite failed.\n");
        }
        else {
            output("response invite successuflly.\n");

            while (session_ctx.unchanged_streams > 0)
                usleep(200);

            rc = ela_session_start(session_ctx.ws, session_ctx.remote_sdp,
                                       session_ctx.sdp_len);

            output("Session start %s.\n", rc == 0 ? "success" : "failed");
        }
    }
    else if ((strcmp(argv[1], "refuse") == 0) && (argc == 3)) {
        rc = ela_session_reply_request(session_ctx.ws, NULL, 1, argv[2]);
        if (rc < 0) {
            output("response invite failed.\n");
        }
        else {
            output("response invite successuflly.\n");
        }
    }
    else {
        output("Unknown sub command.\n");
        return;
    }
}

static void stream_write(ElaCarrier *w, int argc, char *argv[])
{
    ssize_t rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_stream_write(session_ctx.ws, atoi(argv[1]),
                              argv[2], strlen(argv[2]) + 1);
    if (rc < 0) {
        output("write data failed.\n");
    }
    else {
        output("write data successfully.\n");
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
    int i;
    char *packet;
    struct bulk_write_args *args = (struct bulk_write_args *)arg;
    struct timeval start, end;
    int duration;
    float speed;

    packet = (char *)alloca(args->packet_size);
    memset(packet, 'D', args->packet_size);

    output("Begin sending data");

    gettimeofday(&start, NULL);
    for (i = 0; i < args->packet_count; i++) {
        size_t total = args->packet_size;
        size_t sent = 0;

        do {
            rc = ela_stream_write(session_ctx.ws, args->stream,
                                      packet + sent, total - sent);
            if (rc == 0) {
                usleep(100);
                continue;
            } else if (rc < 0) {
                if (ela_get_error() == ELA_GENERAL_ERROR(ELAERR_BUSY)) {
                    usleep(100);
                    continue;
                } else {
                    output("\nWrite data failed.\n");
                    return NULL;
                }
            }

            sent += rc;
        } while (sent < total);

        if (i % 1000 == 0)
            output(".");
    }
    gettimeofday(&end, NULL);

    duration = (int)((end.tv_sec - start.tv_sec) * 1000000 +
                     (end.tv_usec - start.tv_usec)) / 1000;
    duration = (duration == 0) ? 1 : duration;
    speed = (float)(((args->packet_size * args->packet_count) / duration) * 1000) / 1024;

    output("\nFinish! Total %" PRIu64 " bytes in %d.%03d seconds. %.2f KB/s\n",
           (uint64_t)(args->packet_size * args->packet_count),
           (int)(duration / 1000), (int)(duration % 1000), speed);

    return NULL;
}

static void stream_bulk_write(ElaCarrier *w, int argc, char *argv[])
{
    pthread_attr_t attr;
    pthread_t th;

    if (argc != 4) {
        output("Invalid command syntax.\n");
        return;
    }

    args.stream = atoi(argv[1]);
    args.packet_size = atoi(argv[2]);
    args.packet_count = atoi(argv[3]);

    if (args.stream <= 0 || args.packet_size <= 0 || args.packet_count <= 0) {
        output("Invalid command syntax.\n");
        return;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&th, &attr, bulk_write_thread, &args);
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

    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_stream_get_transport_info(session_ctx.ws, atoi(argv[1]), &info);
    if (rc < 0) {
        output("get remote addr failed.\n");
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

static void stream_add_channel(ElaCarrier *w, int argc, char *argv[])
{
    int ch;

    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    ch = ela_stream_open_channel(session_ctx.ws, atoi(argv[1]), NULL);
    if (ch <= 0) {
        output("Create channel failed.\n");
    } else {
        output("Channel %d created.\n", ch);
    }
}

static void stream_close_channel(ElaCarrier *w, int argc, char *argv[])
{
    int rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_stream_close_channel(session_ctx.ws, atoi(argv[1]), atoi(argv[2]));
    if (rc < 0) {
        output("Close channel failed.\n");
    } else {
        output("Channel %s closed.\n", argv[2]);
    }
}

static void stream_write_channel(ElaCarrier *w, int argc, char *argv[])
{
    ssize_t rc;

    if (argc != 4) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_stream_write_channel(session_ctx.ws, atoi(argv[1]),
            atoi(argv[2]), argv[3], strlen(argv[3])+1);
    if (rc < 0) {
        output("Write channel failed.\n");
    } else {
        output("Channel %s write successfully.\n", argv[2]);
    }
}

static void stream_pend_channel(ElaCarrier *w, int argc, char *argv[])
{
    ssize_t rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_stream_pend_channel(session_ctx.ws, atoi(argv[1]),
                                     atoi(argv[2]));
    if (rc < 0) {
        output("Pend channel(input) failed.\n");
    } else {
        output("Channel %s input is pending.\n", argv[2]);
    }
}

static void stream_resume_channel(ElaCarrier *w, int argc, char *argv[])
{
    ssize_t rc;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    rc = ela_stream_resume_channel(session_ctx.ws, atoi(argv[1]),
                                       atoi(argv[2]));

    if (rc < 0) {
        output("Resume channel(input) failed.\n");
    } else {
        output("Channel %s input is resumed.\n", argv[2]);
    }
}

static void session_add_service(ElaCarrier *w, int argc, char *argv[])
{
    PortForwardingProtocol protocol;
    int rc;

    if (argc != 5) {
        output("Invalid command syntax.\n");
        return;
    }

    if (strcmp(argv[2], "tcp") == 0)
        protocol = PortForwardingProtocol_TCP;
    else {
        output("Unknown protocol %s.\n", argv[2]);
        return;
    }

    rc = ela_session_add_service(session_ctx.ws, argv[1],
                                     protocol, argv[3], argv[4]);
    output("Add service %s %s.\n", argv[1], rc == 0 ? "success" : "failed");
}

static void session_remove_service(ElaCarrier *w, int argc, char *argv[])
{
    if (argc != 2) {
        output("Invalid command syntax.\n");
        return;
    }

    ela_session_remove_service(session_ctx.ws, argv[1]);
    output("Service %s removed.", argv[1]);
}

static void portforwarding_open(ElaCarrier *w, int argc, char *argv[])
{
    PortForwardingProtocol protocol;
    int pfid;

    if (argc != 6) {
        output("Invalid command syntax.\n");
        return;
    }

    if (strcmp(argv[3], "tcp") == 0)
        protocol = PortForwardingProtocol_TCP;
    else {
        output("Unknown protocol %s.\n", argv[3]);
        return;
    }

    pfid = ela_stream_open_port_forwarding(session_ctx.ws, atoi(argv[1]),
                                argv[2], protocol, argv[4], argv[5]);

    if (pfid > 0) {
        output("Open portforwarding to service %s <<== %s:%s success, id is %d.\n",
               argv[2], argv[4], argv[5], pfid);
    } else {
        output("Open portforwarding to service %s <<== %s:%s failed.\n",
               argv[2], argv[4], argv[5]);
    }
}

static void portforwarding_close(ElaCarrier *w, int argc, char *argv[])
{
    int pfid;

    if (argc != 3) {
        output("Invalid command syntax.\n");
        return;
    }

    pfid = atoi(argv[2]);
    if (pfid <= 0) {
        output("Invalid portforwarding id %s.\n", argv[2]);
        return;
    }

    pfid = ela_stream_close_port_forwarding(session_ctx.ws, atoi(argv[1]), pfid);
    output("Portforwarding %d closed.\n", pfid);
}

static void help(ElaCarrier *w, int argc, char *argv[]);

struct command {
    const char *cmd;
    void (*function)(ElaCarrier *w, int argc, char *argv[]);
    const char *help;
} commands[] = {
    { "help",       help,                   "help - Display available command list. *OR* help [Command] - Display usage description for specific command." },
    { "clear",      clear_screen,           "clear - Clear log and output view in shell. *OR* clear [log | out] - Clear log or output view in shell." },

    { "address",    get_address,            "address - Display own address." },
    { "nodeid",     get_nodeid,             "nodeid - Display own node ID." },
    { "userid",     get_userid,             "userid - Display own user ID." },
    { "me",         self_info,              "me - Display own details. *OR* me set [name | description | gender | phone | email | region] [Value] - Set own user details individually." },
    { "nospam",     self_nospam,            "nospam - Display current nospam value. *OR* nospam [ value ] - Change nospam value to enforce address change." },
    { "presence",   self_presence,          "presence - Display current presence. *OR* presence [ none | away | busy ] - Display self presence." },

    { "fadd",       friend_add,             "fadd [Address] [Message] - Add new friend." },
    { "faccept",    friend_accept,          "faccept [User ID] - Accept friend request." },
    { "fremove",    friend_remove,          "fremove [User ID] - Remove friend." },
    { "friends",    list_friends,           "friends - List all friends." },
    { "friend",     show_friend,            "friend [User ID] - Display friend details." },
    { "label",      label_friend,           "label [User ID] [Name] - Add label to friend." },
    { "msg",        send_message,           "msg [User ID] [Message] - Send message to a friend." },
    { "bulkmsg",    send_bulk_message,      "bulkmsg [User ID] [Count] [Message] - Send numerous messages to a friend." },
    { "ackmsg",     send_receipt_message,   "ackmsg [User ID] [Count] [Message] - Send messages with receipt to a friend." },
    { "ackbmsg",    send_receipt_bulkmessage,   "ackbmsg [User ID] [Count] [Message] - Send messages with receipt to a friend." },
    { "bigmsgbenchmarkinit", bigmsg_benchmark_initialize, "bigmsgbenchmarkinit [User ID] [Count] - Initialize a big message benchmark to send [count]MB big message to a friend." },
    { "bigmsgbenchmarkacpt", bigmsg_benchmark_accept, "bigmsgbenchmarkacpt - Accept a big message benchmark initialized by a friend." },
    { "bigmsgbenchmarkrej",  bigmsg_benchmark_reject, "bigmsgbenchmarkrej - Reject a big message benchmark initialized by a friend." },
    { "invite",     invite,                 "invite [User ID] [Message] [Bundle] - Invite friend." },
    { "ireply",     reply_invite,           "ireply [User ID] confirm [Message] [Bundle] *OR* ireply [User ID] refuse [Reason] [Bundle] - Confirm or refuse invitation with a message or reason." },

    { "gnew",       group_new,              "gnew - Create new group." },
    { "gleave",     group_leave,            "gleave [Group ID] - Leave group." },
    { "ginvite",    group_invite,           "ginvite [Group ID] [User ID] - Invite user to group." },
    { "gjoin",      group_join,             "gjoin [User ID] cookie - Group invitation from user with cookies." },
    { "gmsg",       group_send_message,     "gmsg [Group ID] [Message] - Send message to group." },
    { "gtitle",     group_set_title,        "gtitle [Group ID] - Display title of group. *OR* gtitle [Group ID] [Title] -  Set title of group." },
    { "gpeers",     group_list_peers,       "gpeers [Group ID] - Display list of participants in group." },
    { "glist",      group_list,             "glist - Display list of joined group." },

    { "sinit",      session_init,           "sinit - Initialize session." },
    { "snew",       session_new,            "snew [User ID] - Start new session with user." },
    { "sadd",       stream_add,             "sadd [plain | reliable | multiplexing | portforwarding] - Add session properties."},
    { "sremove",    stream_remove,          "sremove [Session ID] - Leave session." },
    { "srequest",   session_request,        "srequest - Send a sesion request." },
    { "sreply",     session_reply_request,  "sreply ok - Accept a session request. *OR* sreply refuse [Reason] - Refuse a session request with a reason."},
    { "swrite",     stream_write,           "swrite [Stream ID] [String]  - Send data to stream." },
    { "sbulkwrite", stream_bulk_write,      "sbulkwrite [Stream ID] [Packet size] [Packet count] -  Send bulk data to stream." },
    { "sbulkrecv",  stream_bulk_receive,    "sbulkrecv [ start | end ] - Start or end receiving in bulk." },
    { "scadd",      stream_add_channel,     "scadd [Stream]  - Add stream channel." },
    { "sinfo",      stream_get_info,        "sinfo [ID] - Display stream information."},
    { "scclose",    stream_close_channel,   "scclose [Stream] channel - Close stream channel." },
    { "scwrite",    stream_write_channel,   "scwrite [Stream] channel [String] - Write to stream channel." },
    { "scpend",     stream_pend_channel,    "scpend [Stream] channel - Display pending stream channels." },
    { "scresume",   stream_resume_channel,  "scresume [Stream] channel - Resume stream." },
    { "sclose",     session_close,          "sclose - Close session." },
    { "spfsvcadd",  session_add_service,    "spfsvcadd [Name] [tcp|udp] [Host] [Port] - Add service to session." },
    { "spfsvcremove", session_remove_service, "spfsvcremove [Name] - Remove service from session." },
    { "spfopen",    portforwarding_open,    "spfopen [Stream] [Service] [tcp|udp] [Host] [Port] - Open portforwarding." },
    { "spfclose",   portforwarding_close,   "spfclose [Stream] [PF ID] - Close portforwarding." },
    { "scleanup",   session_cleanup,        "scleanup - Cleanup session." },
    { "kill",       kill_carrier,           "kill - Stop carrier." },
    { NULL }
};

static void help(ElaCarrier *w, int argc, char *argv[])
{
    char line[256] = "\x0";
    size_t len = 0;
    size_t cmd_len;
    struct command *p;

    if (argc == 1) {
        output(" Use *help [Command]* to see usage description for a specific command.\n Available commands list:\n");

        for (p = commands; p->cmd; p++) {
            cmd_len = strlen(p->cmd);
            if (len + cmd_len + 1 > (size_t)OUTPUT_COLS - 2) {
                output("  %s\n", line);
                strcpy(line, p->cmd);
                strcat(line, " ");
                len = cmd_len + 1;
            } else {
                strcat(line, p->cmd);
                strcat(line, " ");
                len += cmd_len + 1;
            }
        }

        if (len > 0)
            output("  %s\n", line);
    } else {
        for (p = commands; p->cmd; p++) {
            if (strcmp(argv[1], p->cmd) == 0) {
                output("Usage: %s\n", p->help);
                return;
            }
        }

        output("Unknown command: %s\n", argv[1]);
    }
}

static void do_cmd(ElaCarrier *w, char *line)
{
    char *args[512];
    int count = 0;
    char *p;
    int word = 0;

    for (p = line; *p != 0; p++) {
        if (isspace(*p)) {
            *p = 0;
            word = 0;
        } else {
            if (word == 0) {
                args[count] = p;
                count++;
            }

            word = 1;
        }
    }

    if (count > 0) {
        struct command *p;

        for (p = commands; p->cmd; p++) {
            if (strcmp(args[0], p->cmd) == 0) {
                p->function(w, count, args);
                return;
            }
        }

        output("Unknown command: %s\n", args[0]);
    }
}

static char *read_cmd(void)
{
    int x, y;
    int w, h;
    int ch = 0;
    int rc;
    char *p;

    static int cmd_len = 0;
    static char cmd_line[1024];

    ch = getch();
    if (ch == -1)
        return NULL;

    getmaxyx(cmd_win, h, w);
    getyx(cmd_win, y, x);

    (void)h;

    pthread_mutex_lock(&screen_lock);
    if (ch == 10 || ch == 13) {
        rc = mvwinnstr(cmd_win, 0, 2, cmd_line, sizeof(cmd_line));
        mvwinnstr(cmd_win, 1, 0, cmd_line + rc, sizeof(cmd_line) - rc);

        wclear(cmd_win);
        waddstr(cmd_win, "# ");
        wrefresh(cmd_win);
        cmd_len = 0;

        // Trim trailing spaces;
        for (p = cmd_line + strlen(cmd_line) - 1; p >= cmd_line && isspace(*p); p--);
        *(++p) = 0;

        // Trim leading spaces;
        for (p = cmd_line; *p && isspace(*p); p++);

        if (strlen(p)) {
            history_add_cmd(p);
            pthread_mutex_unlock(&screen_lock);
            return p;
        }

    } else if (ch == 127) {
        if (cmd_len > 0 && y * w + x - 2 > 0) {
            if (x == 0) {
                x = w;
                y--;
            }
            wmove(cmd_win, y, x-1);
            wdelch(cmd_win);
            cmd_len--;
        }
    } else if (ch == 27) {
        getch();
        ch = getch();
        if (ch == 65 || ch == 66) {
            p = ch == 65 ? (char *)history_prev() : (char *)history_next();
            wclear(cmd_win);
            waddstr(cmd_win, "# ");
            if (p) waddstr(cmd_win, p);
            cmd_len = p ? (int)strlen(p) : 0;
        } /* else if (ch == 67) {
            if (y * w + x - 2 < cmd_len) {
                if (x == w-1) {
                    x = -1;
                    y++;
                }
                wmove(cmd_win, y, x+1);
            }
        } else if (ch == 68) {
            if (y * w + x - 2 > 0) {
                if (x == 0) {
                    x = w;
                    y--;
                }
                wmove(cmd_win, y, x-1);
            }
        }
        */
    } else {
        if (y * w + x - 2 >= cmd_len) {
            waddch(cmd_win, ch);
        } else {
            winsch(cmd_win, ch);
            wmove(cmd_win, y, x+1);
        }

        cmd_len++;
    }

    wrefresh(cmd_win);
    pthread_mutex_unlock(&screen_lock);

    return NULL;
}

static void idle_callback(ElaCarrier *w, void *context)
{
    char *cmd = read_cmd();

    if (cmd)
        do_cmd(w, cmd);
}

static void connection_callback(ElaCarrier *w, ElaConnectionStatus status,
                                void *context)
{
    switch (status) {
    case ElaConnectionStatus_Connected:
        output("Connected to carrier network.\n");
        break;

    case ElaConnectionStatus_Disconnected:
        output("Disconnect from carrier network.\n");
        break;

    default:
        output("Error!!! Got unknown connection status %d.\n", status);
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
    switch (status) {
    case ElaConnectionStatus_Connected:
        output("Friend[%s] connection changed to be online\n", friendid);
        break;

    case ElaConnectionStatus_Disconnected:
        if (bigmsg_benchmark.state != IDLE && !strcmp(friendid, bigmsg_benchmark.peer))
            bigmsg_benchmark.state = IDLE;
        output("Friend[%s] connection changed to be offline.\n", friendid);
        break;

    default:
        output("Error!!! Got unknown connection status %d.\n", status);
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
    output("Friend request from user[%s] with HELLO: %s.\n",
           *info->name ? info->name : userid, hello);
    output("Reply use following commands:\n");
    output("  faccept %s\n", userid);
}

static void message_callback(ElaCarrier *w, const char *from,
                             const void *msg, size_t len,
                             int64_t timestamp, bool is_offline,
                             void *context)
{
    char ctlsig_type[128];
    size_t totalsz;
    int rc;

    rc = sscanf(msg, "bigmsgbenchmark %128s %zu", ctlsig_type, &totalsz);
    if (rc < 1) {
        if (bigmsg_benchmark.state != ONGOING || strcmp(from, bigmsg_benchmark.peer)) {
            output("Message(%s) from friend[%s] at %lld: (%d)%.*s\n", is_offline ? "offline" : "online", from, timestamp, (int)len, (int)len, (const char *)msg);
            return;
        }

        bigmsg_benchmark.rcvd += len;
        if (bigmsg_benchmark.rcvd == bigmsg_benchmark.totalsz)
            gettimeofday(&bigmsg_benchmark.dl_end, NULL);
        return;
    }

    if (!strcmp(ctlsig_type, "request")) {
        if (rc != 2) {
            output("Invalid bigmsgbenchmark request signal: %s\n", msg);
            return;
        }

        if (bigmsg_benchmark.state != IDLE) {
            output("Received a bigmsgbenchmark request signal when we are busy, ignore\n");
            bigmsg_benchmark_send_reject(w, from);
            return;
        }

        memset(&bigmsg_benchmark, 0, sizeof(bigmsg_benchmark));
        bigmsg_benchmark.role = RESPONDER;
        bigmsg_benchmark.state = PENDING;
        strcpy(bigmsg_benchmark.peer, from);
        bigmsg_benchmark.totalsz = totalsz << 20;

        output("Received a bigmsgbenchmark request from [%s], transfer size: %zuMb\n",
                from, totalsz);
        output("Input [bigmsgbenchmarkacpt] to accept, [bigmsgbenchmarkrej] to reject\n");
    } else if (!strcmp(ctlsig_type, "accept")) {
        pthread_attr_t attr;
        pthread_t th;

        if (bigmsg_benchmark.state != PENDING || bigmsg_benchmark.role != INITIATOR ||
            strcmp(from, bigmsg_benchmark.peer)) {
            output("Received a bigmsgbenchmark accept in wrong state, ignore\n");
            return;
        }

        output("Bigmessage benchmark accepted by peer, start benchmark.\n");
        bigmsg_benchmark.state = ONGOING;
        gettimeofday(&bigmsg_benchmark.start, NULL);

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&th, &attr, bigmsg_benchmark_write_thread, w);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&th, &attr, monitor_bigmsg_benchmark_progress, NULL);
        pthread_attr_destroy(&attr);
    } else if (!strcmp(ctlsig_type, "reject")) {
        if (bigmsg_benchmark.state != PENDING || bigmsg_benchmark.role != INITIATOR ||
            strcmp(from, bigmsg_benchmark.peer)) {
            output("Received a bigmsgbenchmark reject in wrong state, ignore\n");
            return;
        }

        output("Bigmessage benchmark is rejected by peer\n");
        bigmsg_benchmark.state = IDLE;
    } else {
        output("Received a invalid bigmsgbenchmark signal, ignore\n");
        return;
    }
}

static void invite_request_callback(ElaCarrier *w, const char *from, const char *bundle,
                                    const void *data, size_t len, void *context)
{
    output("Invite request from[%s] with data: %.*s and bundle: %s\n", from,
           (int)len, (const char *)data, bundle ? bundle : "N/A");
    output("Reply use following commands:\n");
    output("  ireply %s confirm [message] [bundle]\n", from);
    output("  ireply %s refuse [reason] [bundle]\n", from);
}

static void sprint_group_invite_cookie_string(const void *cookie, size_t len,
        char *cookie_str)
{
    size_t i;

    for (i = 0; i < len; i++, cookie_str += 2) {
        sprintf(cookie_str, "%02hhX", ((uint8_t *)cookie)[i]);
    }
}

static void group_invite_request_callback(ElaCarrier *w, const char *from,
                                          const void *cookie, size_t len,
                                          void *context)
{
    char *cookie_str;

    cookie_str = alloca((len << 1) + 1);

    sprint_group_invite_cookie_string(cookie, len, cookie_str);
    output("Group invite request from[%s] with cookie[%s].\n", from, cookie_str);
    output("Input [gjoin %s %s] to join in.\n", from, cookie_str);
}

static void group_connected_callback(ElaCarrier *carrier, const char *groupid,
                                     void *context)
{
    output("Group[%s] has connected\n", groupid);
}

static void group_message_callback(ElaCarrier *carrier, const char *groupid,
                                   const char *from, const void *message,
                                   size_t length, void *context)
{
    output("Group[%s] message from peer[%s]: %.*s\n", groupid, from,
           (int)length, (const char *)message);
}

static void group_title_callback(ElaCarrier *carrier, const char *groupid,
                                 const char *from, const char *title,
                                 void *context)
{
    output("Group[%s] title changed to [%s] by peer[%s]\n",
           groupid, title, from);
}

static void group_peer_name_callback(ElaCarrier *carrier, const char *groupid,
                                     const char *peerid, const char *peer_name,
                                     void *context)
{
    output("Group[%s] peer[%s]'s name changed to %s\n", groupid,
           peerid, peer_name);
}

void group_peer_list_changed_callback(ElaCarrier *carrier, const char *groupid,
                                      void *context)
{
    int peer_number = 0;

    output("Group[%s] peer list changed to:\n", groupid);
    (void)ela_group_get_peers(carrier, groupid, print_group_peer_info,
                              (void *)&peer_number);
}

static void usage(void)
{
    printf("Elastos shell, an interactive console client application.\n");
    printf("Usage: elashell [OPTION]...\n");
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

void signal_handler(int signum)
{
    cleanup_screen();
    history_save();
    exit(-1);
}

int main(int argc, char *argv[])
{
    const char *config_file = NULL;
    ElaCarrier *w;
    ElaOptions opts;
    int wait_for_attach = 0;
    char buf[ELA_MAX_ADDRESS_LEN+1];
    ElaCallbacks callbacks;
    int rc;

    int opt;
    int idx;
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

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    memset(&opts, 0, sizeof(opts));

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
#ifndef _MSC_VER
        printf("After debugger attached, press any key to continue......");
        getchar();
#else
        DebugBreak();
#endif
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGSEGV, signal_handler);
#if !defined(_WIN32) && !defined(_WIN64)
    signal(SIGKILL, signal_handler);
    signal(SIGHUP, signal_handler);
#endif

    config_file = get_config_file(config_file, default_config_files);
    if (!config_file) {
        fprintf(stderr, "Error: Missing config file.\n");
        usage();
        return -1;
    }

    if (!carrier_config_load(config_file, NULL, &opts)) {
        fprintf(stderr, "loading configure failed !\n");
        return -1;
    }

    carrier_config_update(&opts, argc, argv);

    opts.log_printer = log_print;
    strcpy(default_data_location, opts.persistent_location);

    init_screen();
    history_load();

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.idle = idle_callback;
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
    callbacks.group_invite = group_invite_request_callback;
    callbacks.group_callbacks.group_connected = group_connected_callback;
    callbacks.group_callbacks.group_message = group_message_callback;
    callbacks.group_callbacks.group_title = group_title_callback;
    callbacks.group_callbacks.peer_name = group_peer_name_callback;
    callbacks.group_callbacks.peer_list_changed = group_peer_list_changed_callback;

    w = ela_new(&opts, &callbacks, NULL);
    carrier_config_free(&opts);
    if (!w) {
        output("Error create carrier instance: 0x%x\n", ela_get_error());
        output("Press any key to quit...");
        nodelay(stdscr, FALSE);
        getch();
        goto quit;
    }

    output("Carrier node identities:\n");
    output("   Node ID: %s\n", ela_get_nodeid(w, buf, sizeof(buf)));
    output("   User ID: %s\n", ela_get_userid(w, buf, sizeof(buf)));
    output("   Address: %s\n\n", ela_get_address(w, buf, sizeof(buf)));
    output("\n");

    rc = ela_run(w, 10);
    if (rc != 0) {
        output("Error start carrier loop: 0x%x\n", ela_get_error());
        output("Press any key to quit...");
        nodelay(stdscr, FALSE);
        getch();
        ela_kill(w);
        goto quit;
    }

quit:
    cleanup_screen();
    history_save();
    return 0;
}

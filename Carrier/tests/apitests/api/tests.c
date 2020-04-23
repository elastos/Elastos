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
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CUnit/Basic.h>
#include <crystal.h>

#include "ela_carrier.h"

#include "config.h"
#include "suites.h"

#define OFFLINE_MSG_TEST_NAME   "friend_offline_message_test"
#define OFFLINE_MSG_TEST_FILE   "friend_offline_message_test.c"

static void shuffle(int *order, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        int rnd = rand() % count;
        if (rnd == i)
            continue;

        int tmp = order[i];
        order[i] = order[rnd];
        order[rnd] = tmp;
    }
}

static SOCKET cmd_sock = INVALID_SOCKET;

static int connect_robot(const char *host, const char *port)
{
    int ntries = 0;
    assert(host && *host);
    assert(port && *port);

    vlogI("Connecting to test robot(%s:%s).", host, port);

    while(ntries < 3) {
        cmd_sock = socket_connect(host, port);
        if (cmd_sock != INVALID_SOCKET) {
#ifdef _WIN32
            struct timeval timeout = {900000,0};//900s
#else
            struct timeval timeout = {900,0};//900s
#endif
            setsockopt(cmd_sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
            setsockopt(cmd_sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
            break;
        }

#ifdef _WIN32
        if (socket_errno() == WSAECONNREFUSED)
#else
        if (socket_errno() == ENODATA)
#endif
        {
            vlogW("Connecting to test rebot failed, try again");
            ntries++;
            sleep(1);
        }
    }

    if(cmd_sock == INVALID_SOCKET) {
        vlogE("Connecting to test robot failed.");
        return -1;
    }

    vlogI("Connected to robot.");
    return 0;
}

static void disconnect_robot(void)
{
    if (cmd_sock != INVALID_SOCKET) {
        socket_close(cmd_sock);
        cmd_sock = INVALID_SOCKET;
        vlogI("Disconnected robot.");
    }
}

int write_cmd(const char *cmd, ...)
{
    va_list ap;
    char cmd_line[1024];

    assert(cmd_sock != INVALID_SOCKET);
    assert(cmd);

    va_start(ap, cmd);
    vsprintf(cmd_line, cmd, ap);
    va_end(ap);

    assert(cmd_line[strlen(cmd_line) - 1] == '\n');
    vlogD("@@@@@@@@ Control command: %.*s", (int)(strlen(cmd_line)-1), cmd_line);

    return send(cmd_sock, cmd_line, (int)strlen(cmd_line), 0);
}

int read_ack(const char *format, ...)
{
    int rc;
    char ch;
    char ack_buffer[4096];
    char *ack_ptr = ack_buffer;
    va_list ap;

    assert(cmd_sock != INVALID_SOCKET);
    assert(format);

    memset(ack_buffer, 0, sizeof(ack_buffer));
    while ((rc = recv(cmd_sock, &ch, 1, 0)) == 1) {
        // Match 0x0A or 0x0D0A
        if (ch == 0x0A)
            break;
        else if (ch == 0x0D)
            continue;
        else
            *ack_ptr++ = ch;
    }

    // ingore errors?!
    if (rc < 0) {
        if (errno == EAGAIN) {
            vlogE("recv error. TIMEOUT.");
        }
        return -1;
    }

    vlogD("@@@@@@@@ Got acknowledge: %s", ack_buffer);

    va_start(ap, format);
    rc = vsscanf(ack_buffer, format, ap);
    va_end(ap);

    return rc;
}

char robotid[ELA_MAX_ID_LEN + 1];
char robotaddr[ELA_MAX_ADDRESS_LEN + 1];

int test_main(int argc, char *argv[])
{
    int opt, retryCount = 0;
    int i, j;
    CU_pSuite pSuite;
    CU_TestInfo *ti;
    int suites_cnt, cases_cnt, fail_cnt;
    int suites_order[64];
    int cases_order[64];
    char ack[128];

    optind = 1;
    while ((opt = getopt(argc, argv, "r:c:-:")) != -1) {
        switch (opt) {
        case 'r':
            retryCount = atoi(optarg);
            break;
        }
    }

    if (connect_robot(global_config.robot.host, global_config.robot.port) < 0)
        return -1;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        disconnect_robot();
        return CU_get_error();
    }

    srand((unsigned int)time(NULL));

    for (i = 0, suites_cnt = 0; suites[i].fileName; i++) {
        if (global_config.exclude_offmsg
            && strcmp(suites[i].fileName, OFFLINE_MSG_TEST_FILE) == 0)
            continue;

        suites_order[suites_cnt] = suites_cnt;
        suites_cnt++;
    }

    if (suites_cnt > 1 && global_config.shuffle)
        shuffle(suites_order, suites_cnt);

    for (i = 0; i < suites_cnt; i++) {
        int suite_idx;

        suite_idx = suites_order[i];
        if (global_config.exclude_offmsg
            && strcmp(suites[suite_idx].strName, OFFLINE_MSG_TEST_NAME) == 0)
            continue;

        pSuite = CU_add_suite(suites[suite_idx].strName,
                              suites[suite_idx].pInit,
                              suites[suite_idx].pClean);
        if (NULL == pSuite) {
            CU_cleanup_registry();
            disconnect_robot();
            return CU_get_error();
        }

        ti = suites[suite_idx].pCases();
        for (cases_cnt = 0; ti[cases_cnt].pName; cases_cnt++)
            cases_order[cases_cnt] = cases_cnt;

        if (cases_cnt > 1 && global_config.shuffle)
            shuffle(cases_order, cases_cnt);

        for (j = 0; j < cases_cnt; j++) {
            if (CU_add_test(pSuite, ti[cases_order[j]].pName,
                            ti[cases_order[j]].pTestFunc) == NULL) {
                CU_cleanup_registry();
                disconnect_robot();
                return CU_get_error();
            }
        }
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_set_test_retry_count(retryCount);

    read_ack("%32s %45s %52s", ack, robotid, robotaddr);
    if (strcmp(ack, "ready") != 0) {
        vlogE("Got wrong state from Test Robot: %s", ack);
        CU_cleanup_registry();
        disconnect_robot();
        return -1;
    }

    vlogI("Got robot ID: %s", robotid);
    vlogI("Got robot address: %s", robotaddr);

    CU_basic_run_tests();

    fail_cnt = CU_get_number_of_tests_failed();
    if (fail_cnt > 0) {
        vlogE("Failure Case: %d\n", fail_cnt);
    }

    CU_cleanup_registry();
    disconnect_robot();

#if defined(_WIN32) || defined(_WIN64)
    // Windows PIPE has no EOF, write a 0xFF indicate end of pipe manually.
    fputc(EOF, stdout);
    fputc(EOF, stderr);
#endif

    fflush(stdout);
    fflush(stderr);

    return fail_cnt;
}

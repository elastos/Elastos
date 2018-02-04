#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <CUnit/Basic.h>

#include "ela_carrier.h"

#include "tests.h"
#include "suites.h"

static void signal_handler(int signum)
{
    printf("Got signal: %d\n", signum);
    exit(-1);
}

static void log_print(const char *format, va_list args)
{
    test_log_vprintf(ElaLogLevel_None, format, args);
}

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

char robotid[ELA_MAX_ID_LEN + 1];
char robotaddr[ELA_MAX_ADDRESS_LEN + 1];

int test_main(int argc, char *argv[])
{
    int i, j;
    CU_pSuite pSuite;
    CU_TestInfo *ti;
    int suites_cnt, cases_cnt;
    int suites_order[64];
    int cases_order[64];
    char ack[128];

    sys_coredump_set(true);

    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    // signal(SIGSEGV, signal_handler);
    
    robot_ctrl("start\n");

    if(CUE_SUCCESS != CU_initialize_registry()){
        return CU_get_error();
    }

    ela_log_init(global_config.tests.loglevel, NULL, log_print);

    srand((unsigned int)time(NULL));

    for (suites_cnt = 0; suites[suites_cnt].fileName; suites_cnt++) {
        suites_order[suites_cnt] = suites_cnt;
    }

    if (suites_cnt > 1 && global_config.shuffle)
        shuffle(suites_order, suites_cnt);

    for(i = 0; i < suites_cnt; i++){
        int suite_idx = suites_order[i];
        pSuite = CU_add_suite(suites[suite_idx].strName,
                              suites[suite_idx].pInit,
                              suites[suite_idx].pClean);
        if(NULL == pSuite){
            CU_cleanup_registry();
            return CU_get_error();
        }

        ti = suites[suite_idx].pCases();
        for (cases_cnt = 0; ti[cases_cnt].pName; cases_cnt++) {
            cases_order[cases_cnt] = cases_cnt;
        }

        if (cases_cnt > 1 && global_config.shuffle)
            shuffle(cases_order, cases_cnt);

        for (j = 0; j < cases_cnt; j++) {
            if (CU_add_test(pSuite, ti[cases_order[j]].pName,
                            ti[cases_order[j]].pTestFunc) == NULL) {
                CU_cleanup_registry();
                return CU_get_error();
            }
        }
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);

    wait_robot_ack("%32s %45s %52s", ack, robotid, robotaddr);
    if (strcmp(ack, "ready") != 0) {
        test_log_error("Got wrong state from Test Robot: %s\n", ack);
        CU_cleanup_registry();
        return -1;
    }

    test_log_info("Got robot ID: %s\n", robotid);
    test_log_info("Got robot address: %s\n", robotaddr);

    CU_basic_run_tests();

    CU_cleanup_registry();

    return CU_get_error();
}

#ifndef __API_TEST_SUITES_H__
#define __API_TEST_SUITES_H__

#include <CUnit/Basic.h>

typedef CU_TestInfo* (*CU_CasesFunc)(void);

typedef struct TestSuite {
    const char* fileName;
    const char* strName;
    CU_CasesFunc pCases;
    CU_InitializeFunc pInit;
    CU_CleanupFunc pClean;
    CU_SetUpFunc pSetUp;
    CU_TearDownFunc pTearDown;
} TestSuite;

#define DECL_TESTSUITE(mod) \
    int mod##_suite_init(void); \
    int mod##_suite_cleanup(void); \
    CU_TestInfo *mod##_get_cases(void);

#define DECL_TESTSUITE_EX(mod) \
    int mod##_suite_init(void); \
    int mod##_suite_cleanup(void); \
    void mod##_case_setup(void); \
    void mod##_case_tear_down(void); \
    CU_TestInfo *mod##_get_cases(void);

#define DEFINE_TESTSUITE(mod) \
    { \
        .fileName = #mod".c", \
        .strName  = #mod, \
        .pCases   = mod##_get_cases, \
        .pInit    = mod##_suite_init, \
        .pClean   = mod##_suite_cleanup, \
        .pSetUp   = NULL, \
        .pTearDown= NULL \
    }

#define DEFINE_TESTSUITE_EX(mod) \
    { \
        .fileName = #mod".c", \
        .strName  = #mod, \
        .pCases   = mod##_get_cases, \
        .pInit    = mod##_suite_init, \
        .pClean   = mod##_suite_cleanup, \
        .pSetUp   = mod##_case_setup, \
        .pTearDown= mod##_case_tear_down \
    }

#define DEFINE_TESTSUITE_NULL \
    { \
        .fileName = NULL, \
        .strName  = NULL, \
        .pCases   = NULL, \
        .pInit    = NULL, \
        .pClean   = NULL, \
        .pSetUp   = NULL, \
        .pTearDown= NULL  \
    }

#include "carrier/suites.h"
#include "session/suites.h"

TestSuite suites[] = {
    DEFINE_CARRIER_TESTSUITES,
    DEFINE_SESSION_TESTSUITES,
    DEFINE_TESTSUITE_NULL
};

#endif /* __API_TEST_SUITES_H__ */

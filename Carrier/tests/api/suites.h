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
#include "filetransfer/suites.h"

TestSuite suites[] = {
    DEFINE_CARRIER_TESTSUITES,
    DEFINE_SESSION_TESTSUITES,
    DEFINE_FILETRANSFER_TESTSUITES,
    DEFINE_TESTSUITE_NULL
};

#endif /* __API_TEST_SUITES_H__ */

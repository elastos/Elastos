/*
 * Copyright (c) 2019 Elastos Foundation
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

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#endif
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#include <crystal.h>

#include <CUnit/Basic.h>

#include "ela_did.h"
#include "loader.h"
#include "suites.h"

#ifdef HAVE_SYS_RESOURCE_H
int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}
#endif

int main(int argc, char *argv[])
{
    TestSuite *ts;
    CU_ErrorCode rc;

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    TestData_Init();
    for (ts = suites; ts->fileName != NULL; ts++) {
        CU_SuiteInfo *si = ts->getSuiteInfo();
        rc = CU_register_nsuites(1, si);

        if(rc != CUE_SUCCESS) {
            CU_cleanup_registry();
            return rc;
        }
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    TestData_Deinit();
    CU_cleanup_registry();

    return CU_get_error();
}

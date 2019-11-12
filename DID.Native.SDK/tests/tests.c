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

static void usage(void)
{
    printf("Did API unit tests.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    TestSuite *ts;
    CU_ErrorCode rc;

    const char *did_file = NULL;
    const char *didbp_file = NULL;
    const char *cred_file = NULL;

    int opt;
    int idx;

    struct option options[] = {
        { "did",            required_argument,  NULL, 'd' },
        { "didbp",          required_argument,  NULL, 'b'},
        { "credential",     required_argument,  NULL, 'c' },
        { "help",           no_argument,        NULL, 'h' },
        { NULL,             0,                  NULL,  0  }
    };

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    while ((opt = getopt_long(argc, argv, "d:b:c:h:r:?", options, &idx)) != -1) {
        switch (opt) {
        case 'd':
            did_file = optarg;
			break;
        case 'c':
            cred_file = optarg;
			break;
        case 'b':
            didbp_file = optarg;
            break;
        case 'h':
        case '?':
            usage();
            return -1;
        }
    }

    if (did_file && load_file(did_file, Load_Doc) == -1)
        return -1;
    if (didbp_file && load_file(didbp_file, Load_Docbp) == -1)
        return -1;
    if (cred_file && load_file(cred_file, Load_Credential) == -1)
        return -1;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

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

    CU_cleanup_registry();

    if(global_did_string)
	    free(global_did_string);
    if(global_cred_string)
        free(global_cred_string);

    return CU_get_error();
}

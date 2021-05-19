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

#include <stdlib.h>

#include <CUnit/Basic.h>

#include <ela_carrier.h>

static void test_id_valid(void)
{
    const char *valid_id   = "3KygiXesrAACKrF9cnWQrcWW5KqSRY9rKzVqkKUsEoWL";
    const char *invalid_id = "lKygiXesrAACKrF9cnWQrcWW5KqSRY9rKzVqkKUsEoWL";

    CU_ASSERT_TRUE(ela_id_is_valid(valid_id));
    CU_ASSERT_FALSE(ela_id_is_valid(invalid_id));
}

static CU_TestInfo cases[] = {
    { "test_id_valid", test_id_valid },
    { NULL, NULL }
};

CU_TestInfo *check_id_test_get_cases(void)
{
    return cases;
}

int check_id_test_suite_init(void)
{
    return 0;
}

int check_id_test_suite_cleanup(void)
{
    return 0;
}

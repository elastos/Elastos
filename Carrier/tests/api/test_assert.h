#ifndef __TESTS_ASSERT_H__
#define __TESTS_ASSERT_H__

#include <CUnit/Basic.h>

#define TEST_ASSERT_TRUE(exp) do {              \
                        CU_ASSERT_TRUE((exp));    \
                        if (!(exp))               \
                            goto cleanup;       \
                    } while(0)

#endif // __TESTS_ASSERT_H__

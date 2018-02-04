#include <stdlib.h>
#include <ela_carrier.h>
#include <CUnit/Basic.h>

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

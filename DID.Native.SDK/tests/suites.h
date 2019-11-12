#include <CUnit/Basic.h>

typedef CU_SuiteInfo* (*SuiteInfoFunc)(void);

typedef struct TestSuite {
    const char* fileName;
    SuiteInfoFunc getSuiteInfo;
} TestSuite;

CU_SuiteInfo* didstore_open_test_suite_info(void);
CU_SuiteInfo* diddoc_fromjson_test_suite_info(void);
CU_SuiteInfo* diddoc_getelem_test_suite_info(void);
CU_SuiteInfo* didstore_store_did_test_suite_info(void);
CU_SuiteInfo* didstore_did_op_test_suite_info(void);
CU_SuiteInfo* did_fromstring_test_suite_info(void);
CU_SuiteInfo* did_getelem_test_suite_info(void);
CU_SuiteInfo* cred_fromjson_test_suite_info(void);
CU_SuiteInfo* cred_getelem_test_suite_info(void);
CU_SuiteInfo* didstore_store_cred_test_suite_info(void);
CU_SuiteInfo* didstore_cred_op_test_suite_info(void);
CU_SuiteInfo* didstore_new_did_test_suite_info(void);
CU_SuiteInfo* didrequest_test_suite_info(void);
CU_SuiteInfo* diddoc_sign_test_suite_info(void);
CU_SuiteInfo* diddoc_arelem_test_suite_info(void);


TestSuite suites[] = {
    { "didstore_open_test.c",                didstore_open_test_suite_info           },
    { "diddoc_fromjson_test.c",              diddoc_fromjson_test_suite_info         },
    { "diddoc_getelem_test.c",               diddoc_getelem_test_suite_info          },
    { "diddoc_arelem_test.c",                diddoc_arelem_test_suite_info           },
    { "didstore_store_did_test.c",           didstore_store_did_test_suite_info      },
    { "didstore_did_op_test.c",              didstore_did_op_test_suite_info         },
    { "did_fromstring_test.c",               did_fromstring_test_suite_info          },
    { "did_getelem_test.c",                  did_getelem_test_suite_info             },
    { "cred_fromjson_test.c",                cred_fromjson_test_suite_info           },
    { "cred_getelem_test.c",                 cred_getelem_test_suite_info            },
    { "didstore_store_cred_test.c",          didstore_store_cred_test_suite_info     },
    { "didstore_cred_op_test.c",             didstore_cred_op_test_suite_info        },
    { "didstore_new_did_test.c ",            didstore_new_did_test_suite_info        },
    { "didrequest_test.c",                   didrequest_test_suite_info              },
    { "diddoc_sign_test.c",                  diddoc_sign_test_suite_info             },
    { NULL,                                  NULL                                    }
};

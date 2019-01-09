#include <CUnit/CUnit.h>

/*
 * Individual suites place their externs here
 */
extern CU_TestInfo cunit_tests[];
extern int cunit_init(void);
extern int cunit_cleanup(void);

CU_SuiteInfo suites[] = {
    { "CUNIT", cunit_init, cunit_cleanup, cunit_tests },
    CU_SUITE_INFO_NULL
};


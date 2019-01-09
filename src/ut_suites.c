#include <CUnit/CUnit.h>

/*
 * Individual suites place their externs here
 */

/* ut_cunit */
extern CU_TestInfo cunit_tests[];
extern int cunit_init(void);
extern int cunit_cleanup(void);

/* ut_tpj */
extern CU_TestInfo tpj_tests[];
extern int tpj_init(void);
extern int tpj_cleanup(void);


CU_SuiteInfo suites[] = {
    { "CUNIT", cunit_init, cunit_cleanup, cunit_tests },
    { "TPJ", tpj_init, tpj_cleanup, tpj_tests },
    CU_SUITE_INFO_NULL
};


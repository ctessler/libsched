#include <CUnit/CUnit.h>

/*
 * Individual suites place their externs here
 */

/* ut_cunit */
extern CU_TestInfo cunit_tests[];
extern int cunit_init(void);
extern int cunit_cleanup(void);

/* ut_task */
extern CU_TestInfo task_tests[];
extern int task_init(void);
extern int task_cleanup(void);

/* ut_task */
extern CU_TestInfo taskset_tests[];
extern int taskset_init(void);
extern int taskset_cleanup(void);

/* ut_tpj */
extern CU_TestInfo tpj_tests[];
extern int tpj_init(void);
extern int tpj_cleanup(void);

/* ut_tpj */
extern CU_TestInfo ordl_tests[];
extern int ut_ordl_init(void);
extern int ut_ordl_cleanup(void);

/* ut_deadlines */
extern CU_TestInfo ut_dl_tests[];
extern int ut_dl_init(void);
extern int ut_dl_cleanup(void);


CU_SuiteInfo suites[] = {
    { "CUNIT", cunit_init, cunit_cleanup, NULL, NULL, cunit_tests },
    { "TASK", task_init, task_cleanup, NULL, NULL, task_tests },
    { "TASKSETS", taskset_init, taskset_cleanup, NULL, NULL, taskset_tests },    
    { "TPJ", tpj_init, tpj_cleanup, NULL, NULL, tpj_tests },
    { "ORDL", ut_ordl_init, ut_ordl_cleanup, NULL, NULL, ordl_tests },
    { "DEADLINES", ut_dl_init, ut_dl_cleanup, NULL, NULL, ut_dl_tests },
    CU_SUITE_INFO_NULL
};


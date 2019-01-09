#include<CUnit/CUnit.h>

static void do_nothing(void);
int cunit_init(void) { return 0; }
int cunit_cleanup(void) { return 0; }

CU_TestInfo cunit_tests[] = {
    { "Verify CUnit Operation", do_nothing},
    CU_TEST_INFO_NULL
};

/**
 * do_nothing test
 *
 * The purpose of this function is to demonstrate a working CUnit test
 */
static void
do_nothing(void) {
	CU_PASS("CUnit registration is successful");
	return;
}

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>

extern CU_SuiteInfo suites[];

int
main(int arcg, char** argv) {
	CU_Suite *suite;
	CU_initialize_registry();

        if (CU_register_suites(suites) != CUE_SUCCESS) {
		fprintf(stderr, "Unable to register suites\n");
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	CU_cleanup_registry();
	return 0;
}

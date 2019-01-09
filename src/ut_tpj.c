#include <CUnit/CUnit.h>
#include <unistd.h>
#include <stdio.h>
#include <libconfig.h>

#include "taskset.h"
#include "taskset-config.h"
#include "tpj.h"

#define EX_ONESPLIT "ex/one_split.ts"

/* Individual tests */
static void ex_exists(void);
static void ex_read(void);
static void ex_one_split(void);

int tpj_init(void) { return 0; }
int tpj_cleanup(void) { return 0; }

CU_TestInfo tpj_tests[] = {
    { "Verify the example files exist", ex_exists},
    { "Read the example file", ex_read},
    { "Run the one_split example", ex_one_split},
    CU_TEST_INFO_NULL
};

/**
 * Verifies the example file used in later tests exists
 */
static void
ex_exists(void) {
	if (access(EX_ONESPLIT, R_OK) == 0) {
		CU_PASS("Example file is readable");
	} else {
		CU_FAIL("Example file is not readable");
	}
}

/**
 * Reads the example file using the task set API
 */
static void
ex_read(void) {
	task_set_t *ts;
	config_t cfg;
	
	config_init(&cfg);
	ts = ts_alloc();

	if (CONFIG_TRUE != config_read_file(&cfg, EX_ONESPLIT)) {
		CU_FAIL("Unable to read example configuration file");
		goto cleanup;
	}
	
	if (!ts_config_process(&cfg, ts)) {
		CU_FAIL("Unable to process configuration file\n");
		goto cleanup;
	}

cleanup:
	ts_destroy(ts);
	config_destroy(&cfg);
}

/**
 * Runs the one split example and verifies the result
 */
static void
ex_one_split(void) {
	task_set_t *ts;
	config_t cfg;
	
	config_init(&cfg);
	ts = ts_alloc();

	if (CONFIG_TRUE != config_read_file(&cfg, EX_ONESPLIT)) {
		CU_FAIL("Unable to read example configuration file");
		goto cleanup;
	}
	
	if (!ts_config_process(&cfg, ts)) {
		CU_FAIL("Unable to process configuration file\n");
		goto cleanup;
	}

	feas_t feas = tpj(ts, 0);

	switch (feas) {
	case FEAS_YES:
		CU_PASS("Feasible, good.");
		break;
	case FEAS_NO:
	case FEAS_MALFORM:
	default:
		CU_FAIL("Infeasible or malformed task set");
		goto cleanup;
	}

	uint32_t count;
	task_link_t *cookie;
	for (count = 0, cookie = ts_first(ts);
	     cookie;
	     cookie = ts_next(ts, cookie), count++) {
	}
	CU_ASSERT_EQUAL(count, 3);

	
cleanup:
	ts_destroy(ts);
	config_destroy(&cfg);

}

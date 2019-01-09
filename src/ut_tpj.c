#include <CUnit/CUnit.h>
#include <unistd.h>
#include <stdio.h>
#include <libconfig.h>

#include "taskset.h"
#include "taskset-config.h"

#define EX_PATH "ex/baruah-2005.ts"

static void ex_exists(void);
static void ex_read(void);

int tpj_init(void) { return 0; }
int tpj_cleanup(void) { return 0; }

CU_TestInfo tpj_tests[] = {
    { "Verify the example file exists", ex_exists},
    { "Read the example file", ex_read},    
    CU_TEST_INFO_NULL
};

/**
 * Verifies the example file used in later tests exists
 */
static void
ex_exists(void) {
	if (access(EX_PATH, R_OK) == 0) {
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

	if (CONFIG_TRUE != config_read_file(&cfg, EX_PATH)) {
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

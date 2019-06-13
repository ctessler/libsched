/*
 * Need a test for the number of tasks in a taskset.
 */
#include <CUnit/CUnit.h>
#include <unistd.h>
#include <stdio.h>
#include <libconfig.h>

#include "taskset-deadlines.h"

int ut_dl_init(void) { return 0; }
int ut_dl_cleanup(void) { return 0; }

static void dl_framework(void);
static void dl_fill_deadlines(void);

CU_TestInfo ut_dl_tests[] = {
    { "Test framework", dl_framework},
    { "Fill deadlines", dl_fill_deadlines},    
    CU_TEST_INFO_NULL
};

/**
 * Verifies the example file used in later tests exists
 */
static void
dl_framework(void) {
	CU_ASSERT_TRUE(1);
}

static void
dl_fill_deadlines(void) {
	task_t *task_a, *task_b;

	task_a = task_alloc(8, 8, 1);
	task_a->wcet(1) = 2;

	task_b = task_alloc(10, 4, 1);
	task_b->wcet(1) = 2;

	task_set_t *ts = ts_alloc();
	ts_add(ts, task_a);
	ts_add(ts, task_b);

	ordl_t head;
	ordl_init(&head);
	ts_fill_deadlines(ts, &head, ts_star(ts));

	or_elem_t *o_cursor = ordl_first(&head);
	CU_ASSERT_TRUE(o_cursor->oe_deadline == 4);
	o_cursor = ordl_next(o_cursor);
	CU_ASSERT_TRUE(o_cursor->oe_deadline == 8);
	
	ordl_clear(&head);
	
	ts_destroy(ts);
}

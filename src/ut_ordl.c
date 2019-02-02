/*
 * Need a test for the number of tasks in a taskset.
 */
#include <CUnit/CUnit.h>
#include <unistd.h>
#include <stdio.h>
#include <libconfig.h>

#include "ordl.h"

int ut_ordl_init(void) { return 0; }
int ut_ordl_cleanup(void) { return 0; }

static void ordl_allocate(void);
static void ordl_elem_basic(void);

CU_TestInfo ordl_tests[] = {
    { "Allocate and deallocate", ordl_allocate},
    { "Basic Element Operation", ordl_elem_basic},
    CU_TEST_INFO_NULL
};

/**
 * Verifies the example file used in later tests exists
 */
static void
ordl_allocate(void) {
	or_elem_t *e = oe_alloc();
	CU_ASSERT_TRUE(e != NULL);
	oe_free(e);
}

static void
ordl_elem_basic(void) {
	or_elem_t *e = oe_alloc();
	task_t *t = task_alloc(10, 5, 1);
	task_link_t* cookie;
	t->wcet(1) = 3;

	ts_add(e->oe_tasks, t);

	cookie = ts_first(e->oe_tasks);
	t = ts_rem(e->oe_tasks, cookie);
	task_free(t);

	oe_free(e);
}

/*
 * Need a test for the number of tasks in a taskset.
 */
#include <CUnit/CUnit.h>
#include <unistd.h>
#include <stdio.h>
#include <libconfig.h>

#include "taskset.h"

/* Individual tests */
static void t_allocate(void);
static void t_star(void);

static void t_add_tasks_8866();

int taskset_init(void) { return 0; }
int taskset_cleanup(void) { return 0; }

CU_TestInfo taskset_tests[] = {
    { "Allocate and deallocate", t_allocate},
    { "T*", t_star},
    CU_TEST_INFO_NULL
};

/**
 * Verifies the example file used in later tests exists
 */
static void
t_allocate(void) {
	task_set_t *ts = NULL;

	ts = ts_alloc();
	CU_ASSERT_TRUE(ts != NULL);
	ts_free(ts);
}

static void
t_star(void) {
        task_set_t *ts = ts_alloc();
	double_t star;

	t_add_tasks_8866(ts);
	star = ts_star(ts);
	CU_ASSERT(star == 8866);

	ts_destroy(ts);
}

/**
 * Adds tasks to the set which should have a T* = 8866
 */
static void
t_add_tasks_8866(task_set_t *ts) {
	task_t *t;
	
	t = task_alloc(200, 180, 1);
	t->wcet(1) = 50;
	ts_add(ts, t);

	t = task_alloc(200, 180, 1);
	t->wcet(1) = 50;
	ts_add(ts, t);

	t = task_alloc(200, 180, 1);
	t->wcet(1) = 50;
	ts_add(ts, t);

	t = task_alloc(202, 180, 1);
	t->wcet(1) = 50;
	ts_add(ts, t);
}

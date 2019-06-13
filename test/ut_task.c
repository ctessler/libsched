#include <CUnit/CUnit.h>
#include <unistd.h>
#include <stdio.h>
#include <libconfig.h>

#include "task.h"

/* Individual tests */
static void task_allocate(void);
static void zero_threads(void);
static void delta_threads(void);
static void util(void);
static void dbf(void);

int task_init(void) { return 0; }
int task_cleanup(void) { return 0; }

CU_TestInfo task_tests[] = {
    { "Allocate and deallocate", task_allocate},
    { "3->1 threads", zero_threads},
    { "3->1->4 threads", delta_threads},
    { "Utilization", util},
    { "DBF", dbf},
    CU_TEST_INFO_NULL
};

/**
 * Verifies the example file used in later tests exists
 */
static void
task_allocate(void) {
	task_t *t = NULL;

	t = task_alloc(30, 20, 3);
	CU_ASSERT_TRUE(t != NULL);
	t->wcet(1) = 10;
	t->wcet(3) = 14;
	t->wcet(3) = 16;

	task_free(t);
	t = NULL;
}

/**
 * Zero threads
 */
static void
zero_threads(void) {
	task_t *t = NULL;

	t = task_alloc(30, 20, 3);
	t->wcet(1) = 10;
	t->wcet(3) = 14;
	t->wcet(3) = 16;

	CU_ASSERT_FALSE(task_threads(t, 0));

	task_free(t);
}

static void
delta_threads(void) {
	task_t *t = NULL;

	t = task_alloc(30, 20, 3);
	t->wcet(1) = 10;
	t->wcet(3) = 14;
	t->wcet(3) = 16;

	CU_ASSERT_TRUE(task_threads(t, 1));
	CU_ASSERT_EQUAL(t->t_threads, 1);
	CU_ASSERT_EQUAL(t->wcet(1), 0);
	t->wcet(1) = 10;
	CU_ASSERT_EQUAL(t->wcet(1), 10);

	task_threads(t, 4);
	CU_ASSERT_EQUAL(t->t_threads, 4);
	CU_ASSERT_EQUAL(t->wcet(1), 0);
	
	
	task_free(t);
}

static void
util(void) {
	task_t *task = NULL;
	float util;

	task = task_alloc(40, 35, 4);
	task->wcet(1) = 10;
	task->wcet(2) = 15;
	task->wcet(3) = 20;
	task->wcet(4) = 25;

	util = 25.0 / 40.0;
	CU_ASSERT_EQUAL(task_util(task), util);

	task->wcet(4) = 22;
	util = 22.0 / 40.0;
	CU_ASSERT_EQUAL(task_util(task), util);
	
	task_free(task);
}

static void
dbf(void) {

}

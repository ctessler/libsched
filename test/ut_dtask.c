#include <CUnit/CUnit.h>
#include <unistd.h>
#include <stdio.h>
#include <libconfig.h>

#include "dag-task.h"

int ut_dtask_init(void) { return 0; }
int ut_dtask_cleanup(void) { return 0; }

static void dtask_allocate(void);
static void dtask_node_alloc(void);
static void dtask_insert_search(void);
static void dtask_w(void);

CU_TestInfo ut_dtask_tests[] = {
    { "Allocate and Deallocate", dtask_allocate},
    { "Allocate and Deallocate Nodes", dtask_node_alloc},
    { "Insert and Search", dtask_insert_search},
    { "Write a file", dtask_w},
    CU_TEST_INFO_NULL
};

/**
 * Verifies the example file used in later tests exists
 */
static void
dtask_allocate(void) {
	dtask_t *task = dtask_alloc("TEST");
	CU_ASSERT_TRUE(task != NULL);
	CU_ASSERT_TRUE(strcmp(task->dt_name, "TEST") == 0);
	dtask_free(task);
}

static void
dtask_node_alloc(void) {
	dnode_t *node = dnode_alloc("n_0");
	CU_ASSERT_TRUE(node != NULL);
	CU_ASSERT_TRUE(strcmp(node->dn_name, "n_0") == 0);
	dnode_free(node);
}

static void
dtask_insert_search(void) {
	dtask_t *task = dtask_alloc("test");
	dnode_t *node = dnode_alloc("n_0");
	node->dn_object = 1;
	node->dn_threads = 2;
	node->dn_wcet_one = 3;
	node->dn_wcet = 4;
	node->dn_factor = 0.5;

	dtask_insert(task, node);
	dnode_free(node);

	node = NULL;
	node = dtask_name_search(task, "n_0");
	CU_ASSERT_TRUE(node != NULL);
	CU_ASSERT_TRUE(node->dn_object = 1);
	CU_ASSERT_TRUE(node->dn_threads = 2);
	CU_ASSERT_TRUE(node->dn_wcet_one = 3);
	CU_ASSERT_TRUE(node->dn_wcet = 4);
	CU_ASSERT_TRUE(node->dn_factor == 0.5);
	CU_ASSERT_TRUE(node->dn_task == task);

	dnode_free(node);
	dtask_free(task);
}

/**
 * Verifies a dag task file can be written
 */
static void
dtask_w(void) {
	FILE *file = fopen("ut-dtask.dot", "w");
	CU_ASSERT_TRUE(file != NULL);

	dtask_t *task = dtask_alloc("TEST");
	dnode_t* node = dnode_alloc("n_0");

	dtask_insert(task, node);
	dtask_write(task, file);
	
	dnode_free(node);
	dtask_free(task);
}

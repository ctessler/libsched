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
static void dtask_insert_remove(void);
static void dtask_ut_insert_edge(void);
static void dtask_w(void);
static void dtask_node_update(void);

CU_TestInfo ut_dtask_tests[] = {
    { "Allocate and Deallocate", dtask_allocate},
    { "Allocate and Deallocate Nodes", dtask_node_alloc},
    { "Insert and Search", dtask_insert_search},
    { "Insert and Remove", dtask_insert_remove},
    { "Insert Edge", dtask_ut_insert_edge},
    { "Write a file", dtask_w},
    { "Update a node", dtask_node_update},
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

static void
dtask_insert_remove(void) {
	dtask_t *task = dtask_alloc("test");
	dnode_t *node = dnode_alloc("n_0");
	node->dn_object = 1;
	node->dn_threads = 2;
	node->dn_wcet_one = 3;
	node->dn_wcet = 4;
	node->dn_factor = 0.5;

	dtask_insert(task, node);
	int rv = dtask_remove(task, node);
	CU_ASSERT_TRUE(rv == 1);

	dnode_free(node);
	node = NULL;

	node = dtask_name_search(task, "n_0");
	CU_ASSERT_TRUE(node == NULL);
	

	dtask_free(task);
}

static void
dtask_ut_insert_edge(void) {
	dtask_t *task = dtask_alloc("test");
	dnode_t *n0 = dnode_alloc("n_0");
	dnode_t *n1 = dnode_alloc("n_1");

	n0->dn_object = 1;
	n0->dn_threads = 2;
	n0->dn_wcet_one = 3;
	n0->dn_wcet = 4;
	n0->dn_factor = 0.5;
	dtask_insert(task, n0);

	n1->dn_object = 6;
	n1->dn_threads = 7;
	n1->dn_wcet_one = 8;
	n1->dn_wcet = 9;
	n1->dn_factor = 1.0;
	dtask_insert(task, n1);
	
	int rv = dtask_insert_edge(task, n0, n1);
	CU_ASSERT_TRUE(rv > 0);

	/* Check that it can be found */
	rv = dtask_insert_edge(task, n0, n1);	
	CU_ASSERT_TRUE(rv == 0);

	
	FILE *file = fopen("ut-example.dot", "w");
	dtask_write(task, file);
	fclose(file);

	dnode_free(n0);
	dnode_free(n1);
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
	fclose(file);
}

static void
dtask_node_update(void) {
	dtask_t *task = dtask_alloc("TEST");
	dnode_t* node = dnode_alloc("n_0");

	dnode_set_object(node, 1);
	dnode_set_threads(node, 2);
	dnode_set_wcet_one(node, 3);
	dnode_set_factor(node, 0.4);

	dtask_insert(task, node);
	dnode_free(node);

	node = NULL;
	node = dtask_name_search(task, "n_0");
	CU_ASSERT_TRUE(node != NULL);
	CU_ASSERT_TRUE(dnode_get_object(node) == 1);
	CU_ASSERT_TRUE(dnode_get_threads(node) == 2);
	CU_ASSERT_TRUE(dnode_get_wcet_one(node) == 3);
	CU_ASSERT_TRUE(dnode_get_factor(node) == (float_t) 0.4);
	CU_ASSERT_TRUE(node->dn_task == task);

	dnode_set_object(node, 5);
	CU_ASSERT_TRUE(node->dn_dirty == 1);
	dnode_update(node);
	CU_ASSERT_TRUE(node->dn_dirty == 0);
	
	dnode_set_threads(node, 6);
	CU_ASSERT_TRUE(node->dn_dirty == 1);
	dnode_update(node);
	CU_ASSERT_TRUE(node->dn_dirty == 0);

	dnode_set_wcet_one(node, 7);
	CU_ASSERT_TRUE(node->dn_dirty == 1);
	dnode_update(node);
	CU_ASSERT_TRUE(node->dn_dirty == 0);	

	dnode_set_factor(node, 0.8);
	CU_ASSERT_TRUE(node->dn_dirty == 1);
	dnode_update(node);
	CU_ASSERT_TRUE(node->dn_dirty == 0);

	FILE *file = fopen("ut-dno.dot", "w");
	dtask_write(task, file);
	fclose(file);
	
	dnode_free(node);
	dtask_free(task);
}

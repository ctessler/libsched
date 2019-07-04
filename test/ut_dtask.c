#include <CUnit/CUnit.h>
#include <unistd.h>
#include <stdio.h>
#include <libconfig.h>

#include "dag-task.h"
#include "dag-walk.h"

int ut_dtask_init(void) { return 0; }
int ut_dtask_cleanup(void) { return 0; }

static void dtask_allocate(void);
static void dtask_node_alloc(void);
static void dtask_insert_search(void);
static void dtask_insert_remove(void);
static void dtask_ut_insert_edge(void);
static void dtask_ut_remove_edge(void);
static void dtask_w(void);
static void dtask_node_update(void);
static void dtask_ut_copy(void);
static void dtask_topo(void);
static void dtask_pathlen(void);

CU_TestInfo ut_dtask_tests[] = {
    { "Allocate and Deallocate", dtask_allocate},
    { "Allocate and Deallocate Nodes", dtask_node_alloc},
    { "Insert and Search", dtask_insert_search},
    { "Insert and Remove", dtask_insert_remove},
    { "Insert Edge", dtask_ut_insert_edge},
    { "Remove Edge", dtask_ut_remove_edge},
    { "Write a file", dtask_w},
    { "Update a node", dtask_node_update},
    { "Copy a DAG task", dtask_ut_copy},
    { "Topological Sort", dtask_topo},
    { "Critical Path Length", dtask_pathlen},
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

	
	dnode_free(n0);
	dnode_free(n1);
	dtask_free(task);

}

static void
dtask_ut_remove_edge(void) {
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
	dedge_t *edge = dtask_search_edge(task, n0->dn_name, n1->dn_name);
	CU_ASSERT_TRUE(edge != NULL);
	dedge_free(edge);

	rv = dtask_remove_edge(task, n0, n1);
	CU_ASSERT_TRUE(rv > 0);

	edge = dtask_search_edge(task, n0->dn_name, n1->dn_name);
	CU_ASSERT_TRUE(edge == NULL);

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
	remove("ut-dtask.dot");
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
	CU_ASSERT_TRUE(node->dn_flags.dirty == 1);
	dnode_update(node);
	CU_ASSERT_TRUE(node->dn_flags.dirty == 0);

	dnode_set_threads(node, 6);
	CU_ASSERT_TRUE(node->dn_flags.dirty == 1);
	dnode_update(node);
	CU_ASSERT_TRUE(node->dn_flags.dirty == 0);

	dnode_set_wcet_one(node, 7);
	CU_ASSERT_TRUE(node->dn_flags.dirty == 1);
	dnode_update(node);
	CU_ASSERT_TRUE(node->dn_flags.dirty == 0);

	dnode_set_factor(node, 0.8);
	CU_ASSERT_TRUE(node->dn_flags.dirty == 1);
	dnode_update(node);
	CU_ASSERT_TRUE(node->dn_flags.dirty == 0);

	dnode_free(node);
	dtask_free(task);
}

static void
dtask_ut_copy(void) {
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

	dtask_insert_edge(task, n0, n1);

	dtask_t *cp = dtask_copy(task);
	CU_ASSERT_TRUE(cp != NULL);

	CU_ASSERT_TRUE(strcmp(cp->dt_name, task->dt_name) == 0);
	dnode_t *nt = dtask_name_search(cp, "n_0");
	CU_ASSERT_TRUE(nt != NULL);
	dnode_free(nt);
	nt = dtask_name_search(cp, "n_1");
	CU_ASSERT_TRUE(nt != NULL);
	dnode_free(nt);

	dedge_t *e = dtask_search_edge(cp, "n_0", "n_1");
	CU_ASSERT_TRUE(e != NULL);
	dedge_free(e);

	dnode_free(n0);
	dnode_free(n1);
	dtask_free(task);
}

static void
dtask_topo(void) {
	char buff[DT_NAMELEN];
	dtask_t *task = dtask_alloc("test");
	dnode_t *nodes[6];
	int marked[6];
	for (int i = 0; i < 6; i++) {
		sprintf(buff, "n_%d", i);
		marked[i] = 0;
		nodes[i] = dnode_alloc(buff);
		dnode_set_wcet_one(nodes[i], i);
		dtask_insert(task, nodes[i]);
	}

	dtask_insert_edge(task, nodes[0], nodes[1]);
	dtask_insert_edge(task, nodes[0], nodes[2]);
	dtask_insert_edge(task, nodes[1], nodes[3]);
	dtask_insert_edge(task, nodes[2], nodes[3]);
	dtask_insert_edge(task, nodes[3], nodes[4]);
	dtask_insert_edge(task, nodes[4], nodes[5]);


	dnode_t **topo = dag_topological(nodes[0]);
	for (int i=0; topo[i] != NULL; i++) {
		marked[dnode_get_wcet_one(topo[i])] = 1;
		dnode_free(topo[i]);
	}

	for (int i=0; i < 6; i++) {
		CU_ASSERT_TRUE(marked[i] == 1);
	}

	for (int i = 0; i < 6; i++) {
		dnode_free(nodes[i]);
	}

	dtask_free(task);
}

static void
dtask_pathlen(void) {
	char buff[DT_NAMELEN];
	dtask_t *task = dtask_alloc("test");
	dnode_t *nodes[6];
	int marked[6];
	for (int i = 0; i < 6; i++) {
		sprintf(buff, "n_%d", i);
		marked[i] = 0;
		nodes[i] = dnode_alloc(buff);
		dnode_set_wcet_one(nodes[i], i+1);
		dnode_set_threads(nodes[i], (i+1)*2);
		dnode_set_object(nodes[i], i);
		dnode_set_factor(nodes[i], .75);
		dtask_insert(task, nodes[i]);
	}

	dtask_insert_edge(task, nodes[0], nodes[1]);
	dtask_insert_edge(task, nodes[0], nodes[2]);
	dtask_insert_edge(task, nodes[1], nodes[3]);
	dtask_insert_edge(task, nodes[2], nodes[3]);
	dtask_insert_edge(task, nodes[3], nodes[4]);
	dtask_insert_edge(task, nodes[4], nodes[5]);
	
	/* After nodes are inserted, they should not be referred to again */
	for (int i = 0; i < 6; i++) {
		dnode_free(nodes[i]);
	}

	/* Calculet the maximum distances to each node */
	dnode_t *source = dtask_source(task);
	dnode_t **topo = dag_maxd(source);
	dnode_free(source);

	/* The final node should have the expected critical path length */
	CU_ASSERT(topo[5]->dn_distance == 137);
	for (int i=0; topo[i] != NULL; i++) {
		dnode_free(topo[i]);
	}
	tint_t cpathlen = dtask_cpathlen(task);
	CU_ASSERT(cpathlen == 137);

	tint_t workload = dtask_workload(task);
	CU_ASSERT(workload == 144);
	
	dtask_free(task);
}


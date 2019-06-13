/*
 * Need a test for the number of tasks in a taskset.
 */
#include <CUnit/CUnit.h>
#include <unistd.h>
#include <stdio.h>
#include <libconfig.h>

#include "ordt.h"

int ut_ordt_init(void) { return 0; }
int ut_ordt_cleanup(void) { return 0; }

static void ordt_allocate(void);
static void ordt_elem_basic(void);
static void ordt_basic_add(void);
static void ordt_add_two(void);
static void ordt_clear(void);
static void ordt_inorder(void);

CU_TestInfo ordt_tests[] = {
    { "Allocate and Deallocate", ordt_allocate},
    { "Basic Element Operation", ordt_elem_basic},
    { "Add One", ordt_basic_add},
    { "Add Two", ordt_add_two},
    { "Clearing the Tree", ordt_clear},
    { "Inorder Walk", ordt_inorder},    
    CU_TEST_INFO_NULL
};

/**
 * Verifies the example file used in later tests exists
 */
static void
ordt_allocate(void) {
	ot_t *tree = ot_alloc();
	CU_ASSERT_TRUE(tree != NULL);
	CU_ASSERT_TRUE(tree->ot_count == 0);
	CU_ASSERT_TRUE(tree->ot_root == NULL);
	ot_free(tree);
}

static void
ordt_elem_basic(void) {
	ot_elem_t *elem = ote_alloc();
	elem->ote_deadline = 3;

	task_t *task = task_alloc(80, 40, 1);
	task->wcet(1) = 5;

	ts_add(elem->ote_tasks, task);

	ote_free(elem);
	CU_ASSERT_TRUE(task != NULL);

	task_free(task);
}

static void
ordt_basic_add(void) {
	ot_t *tree = ot_alloc();
	ot_elem_t *elem = ote_alloc();
	ot_elem_t *dup = ote_alloc();
	elem->ote_deadline = 5;
	dup->ote_deadline = 5;

	int e = ot_ins(tree, elem);
	CU_ASSERT_TRUE(e == 0);
	CU_ASSERT_TRUE(tree->ot_count == 1);

	e = ot_ins(tree, dup);
	CU_ASSERT_TRUE(e != 0);
	CU_ASSERT_TRUE(tree->ot_count == 1);	

	e = ot_rem(tree, elem);
	CU_ASSERT_TRUE(e == 0);
	CU_ASSERT_TRUE(tree->ot_count == 0);		
	
	ote_free(elem);
	ote_free(dup);
	ot_free(tree);
}

static void
ordt_add_two(void) {
	ot_t *tree = ot_alloc();
	ot_elem_t *one = ote_alloc();
	ot_elem_t *two = ote_alloc();
	one->ote_deadline = 5;
	two->ote_deadline = 10;

	int e = ot_ins(tree, one);
	CU_ASSERT_TRUE(e == 0);
	CU_ASSERT_TRUE(tree->ot_count == 1);

	e = ot_ins(tree, one);
	CU_ASSERT_TRUE(e == 0);
	CU_ASSERT_TRUE(tree->ot_count == 2);

	ot_elem_t *needle = ot_find(tree, one->ote_deadline);
	CU_ASSERT_TRUE(needle == one);

	e = ot_rem(tree, one);
	CU_ASSERT_TRUE(e == 0);
	CU_ASSERT_TRUE(tree->ot_count == 1);		

	e = ot_rem(tree, two);
	CU_ASSERT_TRUE(e == 0);
	CU_ASSERT_TRUE(tree->ot_count == 0);		

	ote_free(one);
	ote_free(two);
	ot_free(tree);
}

static void
ordt_clear(void) {
	ot_t *tree = ot_alloc();

	for (int i=0; i < 10; i++) {
		ot_elem_t *elem = ote_alloc();
		elem->ote_deadline = i * 5;

		int e = ot_ins(tree, elem);
		CU_ASSERT_TRUE(e == 0);
	}

	ot_empty(tree);
	/* Should be safe */
	ot_empty(tree);

	ot_free(tree);
}

static void
print_inorder(const void *cookie, const VISIT which, const int depth) {
	ot_elem_t *elem = *(ot_elem_t**) cookie;

	switch (which) {
	case postorder:
		break;
	default:
		return;
	}

	printf("%lu ", elem->ote_deadline);
	       
}

static void
ordt_inorder(void) {
	ot_t *tree = ot_alloc();

	for (int i=0; i < 10; i++) {
		ot_elem_t *elem = ote_alloc();
		elem->ote_deadline = i * 5;

		int e = ot_ins(tree, elem);
		CU_ASSERT_TRUE(e == 0);
	}

	twalk(tree->ot_root, print_inorder);
	printf("\n");
	
	ot_empty(tree);
	/* Should be safe */
	ot_free(tree);
}

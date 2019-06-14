#ifndef ORDT_H
#define ORDT_H

#include <stdint.h>
#include <stdlib.h>
#include <search.h>
#include "taskset.h"

/**
 * @file ordt.h Ordered (Tree) Absolute Deadline Management
 */

typedef struct {
	void *ot_root;	/**< Root of the tree */
	tint_t ot_count;	/**< Number of deadlines in the tree */
} ot_t;

typedef struct ot_elem {
	tint_t ote_deadline;	/**< Absolute deadline */
	task_set_t* ote_tasks;	/**< List of tasks which share the deadline */
} ot_elem_t;

/**
 * Allocates and releases a tree (root)
 *
 * Usage:
 *     ot_t *tree = ot_alloc();
 *     ...
 *     ot_free(tree);
 */
static inline ot_t *ot_alloc(void) {
	ot_t *tree = calloc(1, sizeof(ot_t));
	tree->ot_count = 0;
	tree->ot_root = NULL;
	
}
static inline void ot_free(ot_t *tree) {
	free(tree);
	tree = NULL;
}

/**
 * Inserts an element into the tree
 *
 * Assumes the user already knows the element is *not* in the tree. If
 * it is already present, an error is returned. 
 *
 * @param[in|out] tree the tree being inserted into
 * @param[in] elem the element being inserted
 *
 * @return 0 upon success, non-zero otherwise
 */
int ot_ins(ot_t *tree, ot_elem_t *elem);

/**
 * Removes an element from the tree
 *
 * @param[in|out] tree the tree 
 * @param[in] elem the element being removed
 *
 * @return 0 upon success, non-zero otherwise
 */
int ot_rem(ot_t *tree, ot_elem_t *elem);


/**
 * Finds an element in the tree by its key (deadline)
 *
 * @param[in] tree tree to search
 * @param[in] deadline
 *
 * @return NULL if not found, a pointer to the element if found
 */
ot_elem_t* ot_find(ot_t *tree, tint_t deadline);

/**
 * Empties all items from the tree
 *
 * Usage:
 *	ot_empty(tree);
 *
 * @param[in|out] tree the tree being emptied
 */
void ot_empty(ot_t *tree);

/**
 * Allocates and releases a tree element
 *
 * Usage:
 *     ot_elem_t *elem = ote_alloc();
 *     ...
 *     ot_free(elem);
 */
static inline ot_elem_t *ote_alloc(void) {
	ot_elem_t *elem = calloc(1, sizeof(ot_elem_t));
	elem->ote_tasks = ts_alloc();

	return elem;
}

/**
 * Releases the tree element
 *
 * This does *not* release the memory of the tasks added to the tree
 * element.
 *
 * @param[in|out] elem the element being released
 */
void ote_free(ot_elem_t *elem);


#endif /* ORDT_H */

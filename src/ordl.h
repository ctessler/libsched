#ifndef ORDL_H
#define ORDL_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/queue.h>
#include "taskset.h"

/**
 * @file ordl.h Ordered Absolute Deadline Management
 */

/* Needed before including taskset.h */
typedef LIST_HEAD(ordl_head, or_elem) ordl_t;

typedef struct or_elem {
	LIST_ENTRY(or_elem) oe_glue;	/**< Queue glue */
	task_set_t* oe_tasks;		/**< List of tasks which share the deadline */
	uint32_t oe_deadline;		/**< Absolute deadline */
} or_elem_t;

/**
 * Initializes the list
 *
 * Usage:
 *     ordl_t *head;
 *     ordl_init(head);
 *
 * @param[in|out] head head of the list, must be a pointer.
 */
#define ordl_init(head) LIST_INIT(head)

#define ordl_empty(head) LIST_EMPTY(head)

/**
 * Adds an element to the head of the list
 *
 * Usage:
 *   ordl_t *head;
 *   ordl_elem_t *elem = oe_alloc();
 *   ordl_insert_head(head, elem);
 *
 * @param[in] head list head
 * @param[in] elem item being added to the head
 */
#define ordl_insert_head(head, elem) LIST_INSERT_HEAD(head, elem, oe_glue)

/**
 * Adds an element before or after another in the list
 *
 * Usage:
 *    ordl_t *head;
 *    ordl_elem_t *elem = oe_alloc();
 *    ordl_insert_head(head, elem);
 *    ordl_insert_after(elem, oe_alloc());
 *    ordl_insert_before(elem, oe_alloc());
 *
 * @param[in] elem the element being added to the list
 * @param[in] onlist the element already on the list
 */
#define ordl_insert_after(onlist, elem) \
	LIST_INSERT_AFTER(onlist, elem, oe_glue)
#define ordl_insert_before(onlist, elem) \
	LIST_INSERT_BEFORE(onlist, elem, oe_glue)

/**
 * Removes an element from the list
 *
 * @param[in] elem the element being removed
 */
#define ordl_remove(elem) LIST_REMOVE(elem, oe_glue)

/**
 * Adds an element to the list according to the value of its absolute deadline.
 *
 * @param[in] head the head of the list
 * @param[in] elem the element being added
 */
int ordl_insert(ordl_t* head, or_elem_t *elem);

/**
 * Foreach element
 *
 * Usage:
 *    or_elem_t *elem, *t;
 *    ordl_t head;
 *
 *    ordl_foreach(head, elem) {
 *        ... do stuff with elem
 *            but don't delete it ...
 *    }
 *
 * @param[in] head head of the list
 * @param[in] elem current element
 */
#define ordl_foreach(head, elem) LIST_FOREACH(elem, head, oe_glue)

/**
 * First element of the list
 * 
 * @param[in] head the head of the list
 */
#define ordl_first(head) LIST_FIRST(head)

/**
 * Next element of the list
 *
 * @param[in] elem the element on the list
 *
 * @return the next element on the list
 */
#define ordl_next(elem) LIST_NEXT(elem, oe_glue)

/**
 * Removes all deadlines of an individual task from the ordered list
 * of absolute deadlines
 *
 * @param[in] task the task
 * @param[in] head the (maybe non-empty) list
 *
 * @return the number of deadlines removed
 */
int ordl_rem_deadlines_task(ordl_t *head, task_t *task);

/**
 * Removes all deadlines from the ordered list, it does *not* free the
 * tasks associated weth the deadlines. 
 *
 * @param[out] head the head of the list to be cleared, will be
 *     ordl_init()'d before returned;
 */
void ordl_clear(ordl_t *head);

/**
 * Finds the element of the deadline
 *
 * @param[in] head list head
 * @param[in] deadline the deadline being searched for
 *
 * @return NULL if not found, the element otherwise.
 */
or_elem_t *ordl_find(ordl_t* head, uint32_t deadline);

/**
 * Allocate a new element
 *
 * Usage:
 *    or_elem_t *item = oe_alloc();
 */
or_elem_t* oe_alloc();
or_elem_t* oe_free(or_elem_t* e);





#endif /* ORDL_H */

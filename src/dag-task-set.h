#ifndef DAG_TASK_SET_H
#define DAG_TASK_SET_H

#include "dag-task.h"
#include <sys/queue.h>

/**
 * dtask_set_t the dag task set
 */
typedef LIST_HEAD(dtask_set, dtask_elem) dtask_set_t;

/**
 * Element of dag task set
 */
typedef struct dtask_elem {
	LIST_ENTRY(dtask_elem) dts_glue;/**< Queue glue */
	dtask_t *dts_task;		/**< Task */
	char dts_path[DT_NAMELEN];	/**< Path to read the task */
	tint_t dts_cores;		/**< Dedicated cores */
	tint_t dts_cands;		/**< Candidate count */
	tint_t dts_high;		/**< True if high utilization */
} dtask_elem_t;


/**
 * Initializes the list
 *
 * Usage:
 *     dag_task_set_t *dts = dts_alloc();
 *     // or
 *     dag_task_set_t dts;
 *     dts_init(&dts);
 * @param[in|out] head head of the list, must be a pointer.
 */
dtask_set_t* dts_alloc();
#define dts_init(head) LIST_INIT(head)
#define dts_empty(head) LIST_EMPTY(head)
void dts_free(dtask_set_t *dts);

/**
 * Adds an element to the head of the list
 *
 * Usage:
 *   dag_task_set_t *head = dts_alloc();
 *   dts_elem_t *elem = dtse_alloc();
 *   dts_insert_head(head, elem);
 *
 * @param[in] head list head
 * @param[in] elem item being added to the head
 */
#define dts_insert_head(head, elem) LIST_INSERT_HEAD(head, elem, dts_glue)

/**
 * Adds an element before or after another in the list
 *
 * Usage:
 *    dag_task_set_t *head = dts_alloc();
 *    dts_elem_t *elem = dtse_alloc();
 *    dts_insert_head(head, elem);
 *    dts_insert_after(elem, dtse_alloc());
 *    dts_insert_before(elem, dtse_alloc());
 *
 * @param[in] elem the element being added to the list
 * @param[in] onlist the element already on the list
 */
#define dts_insert_after(onlist, elem) \
	LIST_INSERT_AFTER(onlist, elem, dts_glue)
#define dts_insert_before(onlist, elem) \
	LIST_INSERT_BEFORE(onlist, elem, dts_glue)

/**
 * Removes an element from the list
 *
 * @param[in] elem the element being removed
 */
#define dts_remove(elem) LIST_REMOVE(elem, dts_glue)

/**
 * Adds an element to the list 
 *
 * @param[in] head the head of the list
 * @param[in] elem the element being added
 */
int dts_insert(dtask_set_t* head, dtask_elem_t *elem);

/**
 * Foreach element
 *
 * Usage:
 *    dts_elem_t *elem, *t;
 *    dag_task_set_t head;
 *
 *    dts_foreach(head, elem) {
 *        ... do stuff with elem
 *            but don't delete it ...
 *    }
 *
 * @param[in] head head of the list
 * @param[in] elem current element
 */
#define dts_foreach(head, elem) LIST_FOREACH(elem, head, dts_glue)

/**
 * First element of the list
 * 
 * @param[in] head the head of the list
 */
#define dts_first(head) LIST_FIRST(head)

/**
 * Next element of the list
 *
 * @param[in] elem the element on the list
 *
 * @return the next element on the list
 */
#define dts_next(elem) LIST_NEXT(elem, dts_glue)

/**
 * Removes all elements from the list
 *
 * @note each element (and dtask_t) is released.
 *
 * @param[out] head the head of the list to be cleared, will be
 *     dts_init()'d before returned;
 */
void dts_clear(dtask_set_t *head);

/**
 * Finds the element of the deadline
 *
 * @param[in] head list head
 * @param[in] name the name of the task
 *
 * @return NULL if not found, the element otherwise.
 */
dtask_elem_t *dts_find(dtask_set_t* head, char *name);

/**
 * Allocate a new element
 *
 * Usage:
 *    dtask_t *task;
 *    ...
 *    dtask_update(task);
 *    dts_elem_t *item = dtse_alloc(task);
 */
dtask_elem_t* dtse_alloc(dtask_t* task);
void dtse_free(dtask_elem_t* e);


/**
 * Utilization of the task set
 */
float_t dts_util(dtask_set_t *dts);

/**
 * Determines if the deadlines and periods of all tasks within the set
 * are equal, ie the tasks are all implicit deadline tasks. 
 *
 * @param[in] dts the dag task set
 *
 * @return non-zero if all tasks are implicit deadline tasks, zero
 * otherwise
 */
int dts_implicit(dtask_set_t *dts);

#endif /* DAG_TASK_SET_H */

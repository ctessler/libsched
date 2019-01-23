#ifndef TASKSET_H
#define TASKSET_H

#include <math.h>
#include "task.h"
#include "search.h"
#include "ordl.h"

/**
 * Feasibility constants for tests
 */
typedef enum {
	FEAS_MALFORM = -1,
	FEAS_YES = 0,
	FEAS_NO
} feas_t;

/**
 * Doubly Linked List wrapping the tasks
 */
typedef struct task_link {
	struct task_link *tl_next;
	struct task_link *tl_prev;
	task_t *tl_task;
} task_link_t;

typedef struct {
	task_link_t *ts_head;
} task_set_t;

task_set_t* ts_alloc();
void ts_free(task_set_t* ts);

/**
 * Frees the storage associated with the taskset and frees the tasks
 * within the set
 *
 */
void ts_destroy(task_set_t* ts);

/**
 * Returns a dynamically allocated string representing the task set,
 * must be free()'d 
 */
char* ts_string(task_set_t *ts);

/**
 * Returns a dynamically allocated string to head the task set,
 * must be free()'d 
 */
char *ts_header(task_set_t *ts);

/**
 * Adding and removing tasks from the task set.
 *
 * When a task is added, a cookie is produced. The cookie is valid
 * until the next call of tset_*. 
 *
 * Usage:
 *
 *   task_link_t *cookie = tset_add(task); // task is now in the set
 *   // Time passes
 *   cookie = tset_find(task);
 *   tset_rem(cookie); // task is no longer in the set
 *   task_free(task);
 */

/**
 * Adds a task to the set
 *
 * @param[in] task the task being added to the set
 *
 * @return a cookie tracking where the task has been added in the set
 */
task_link_t* ts_add(task_set_t *ts, task_t *task);

/**
 * Finds a task in the set
 *
 * @param[in] task the task to search for
 *
 * @return a cookie if the task is in the set, NULL otherwise
 */
task_link_t* ts_find(task_set_t *ts, task_t *task);

/**
 * Removes a task from the set
 *
 * @param[in] cookie the cookie of the task in the set
 *
 * @return the task if the cookie was valid and the task was in the
 * set, NULL otherwise
 */
task_t* ts_rem(task_set_t *ts, task_link_t *cookie);

/** 
 * Finds the last task in the set
 *
 * @return a cookie for the last task in the set
 */
task_link_t* ts_last(task_set_t *ts);

/** 
 * Finds the first task in the set
 *
 * @return a cookie for the last task in the set
 */
task_link_t* ts_first(task_set_t *ts);

/**
 * Finds the next task in the set
 *
 * @return a cookie for the next task, NULL otherwise
 */
task_link_t* ts_next(task_set_t *ts, task_link_t *cookie);

/**
 * Gets the task of the cookie
 *
 * Usage:
 *     task_link_t *cookie;
 *     task_t *t = ts_task(cookie);
 *     ts_task(cookie) = ts;
 * 
 * @param[in] cookie the cookie of the task
 *
 * @return pointer to the task (can be assigned)
 */
#define ts_task(x) ((*x).tl_task)

/**
 * Calculates the hyperperiod of the task set
 *
 * @param[in] ts the task set
 *
 * @return the hyperperiod
 */
uint32_t ts_hyperp(task_set_t *ts);

/**
 * Finds the greatest deadline among all tasks
 *
 * @param[in] ts the task set
 * 
 * @return the greatest relative deadline within the set 
 */
uint32_t ts_dmax(task_set_t *ts);

/**
 * Finds the utilization of the task set
 *
 * @param[in] ts the task set
 *
 * @return the utilization of the task set
 */
float_t ts_util(task_set_t *ts);

/**
 * Calculates an upper bound on the value of any deadline to check for
 * schedulability 
 *
 * T*(tasks) = min(P, max( d_max, 1/(1 - U) * sum( U_i * (p_i - d_i))))
 */
uint64_t ts_star(task_set_t *ts);
uint64_t ts_star_debug(task_set_t *ts, FILE *f);

/**
 * Calculates the largest difference between period and deadline of
 * all tasks within the task set 
 *
 * @param[in] ts the task set
 *
 * @return largest difference between period and deadline of any task
 * in the set
 */
int32_t ts_max_pdiff(task_set_t * ts);

/**
 * Determines if a task set is permissible
 *
 * @note Returned string must be free()'d
 *
 * @param[in] ts task set
 *
 * @return NULL if the task set is acceptable, a string describing why the task
 * set is unnacceptable otherwise.
 */
char* ts_permit(task_set_t* ts);

/**
 * Finds the slack at time t for the task set
 *
 * @note the difference between the slack among all deadlines up to
 * and including t (which is what this function calculates) and the
 * demand at a specific time (which is what ts_demand() calculates) 
 *
 * @param[in] ts the task set
 * @param[in] t the time
 *
 * @return the slack, can be negative
 */
int64_t ts_slack(task_set_t *ts, uint32_t t);

/**
 * Finds the specific slack for a single deadline
 *
 * @note the difference between the slack among all deadlines up to
 * and including t (which is what ts_slack() calculates) and the
 * demand at a specific time (which is what this function
 * calculates)
 *
 * @param[in] ts the task set
 * @param[in] t the time
 * 
 * @return the demand at time t
 */
int64_t ts_demand(task_set_t *ts, uint32_t t);
int64_t ts_demand_debug(task_set_t *ts, uint32_t t, FILE *f);

/**
 * Adds all of the deadlines from each of the tasks to the ordered
 * absolute deadline list up to and including deadlines at t
 *
 * @param[in] ts the populated task set
 * @param[in] head the empty list
 * @param[in] t the time bound (deadlines <= t are included)
 *
 * @return non-zero upon success, zero otherwise
 */
int ts_fill_deadlines(task_set_t *ts, ordl_t *head, uint32_t t);

/**
 * Adds all of the deadlines from an individual task to the ordered
 * absolute deadline list up to and including deadlines at t
 *
 * @param[in] task the task
 * @param[in] head the (maybe non-empty) list
 * @param[in] t the time bound
 *
 * @return the number of deadlines added to head
 */
int ts_fill_deadlines_task(task_t *task, ordl_t *head, uint32_t t);

/**
 * Adds new deadlines for all tasks to the set of ordered deadlines.
 *
 * @param[in] ts the set of tasks
 * @param[in] head the head of the ordered absolute deadlines
 * @param[in] prevb the previous time value bound, which new deadlines
 *     must be strictly greater than to be added.
 * @param[in] newb the new time value bound, which new deadlines must
 *     be less than **OR** equal to, to be added to the list.
 *
 * @return the number of deadlines added.
 */
int ts_extend_deadlines(task_set_t *ts, ordl_t *head, uint32_t prevb,
    uint32_t newb);

/**
 * Returns the number of tasks in the task set
 *
 * @note this is an O(n) walk.
 *
 * @param[in] ts the task set
 *
 * @return the number of tasks in the set
 */
uint32_t ts_count(task_set_t *ts);

#endif /* TASKSET_H */

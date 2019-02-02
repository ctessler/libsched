#ifndef TASKSET_DEADLINES_H
#define TASKSET_DEADLINES_H
#include "taskset.h"
#include "ordl.h"

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

#endif /* TASKSET_DEADLINES_H */

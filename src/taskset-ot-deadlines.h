#ifndef TASKSET_OT_DEADLINES_H
#define TASKSET_OT_DEADLINES_H
#include "taskset.h"
#include "ordt.h"

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
tint_t ts_fill_ot_deadlines(task_set_t *ts, ot_t *head, tint_t t);
tint_t ts_fill_ot_deadlines_dbg(task_set_t *ts, ot_t *head, tint_t t, FILE *dbg);

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
tint_t ts_fill_ot_deadlines_task(task_t *task, ot_t *head, tint_t t);
tint_t ts_fill_ot_deadlines_task_dbg(task_t *task, ot_t *head, tint_t t, FILE *dbg);

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
tint_t ts_extend_ot_deadlines(task_set_t *ts, ot_t *head, tint_t prevb,
    tint_t newb);

#endif /* TASKSET_DEADLINES_H */

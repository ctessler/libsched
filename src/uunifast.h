#ifndef UUNIFAST_H
#define UUNIFAST_H
#include <stdio.h>
#include <gsl/gsl_rng.h>
#include "taskset.h"

/**
 * A callback for uunifast_cb() invoked when setting the utilization
 * for a task. This allows the caller to interpret utilization as they
 * see fit, adjusting period or deadline, or other parameters.
 *
 * @param[in] ts the task set the task belongs to
 * @param[in] t the task
 * @param[in] util the utilization being assigned nto the task
 *
 * @return non-zero upon success, zero otherwise
 */
typedef int (*uu_updater)(task_set_t *ts, task_t *t, double util);

/**
 * Implementation of UUniFast "Biasing Effects in Schedulability",
 *   by Bini & Buttazo
 *
 * Modifies **either** the period or the WCET of a task
 *
 * @param[in|out] ts the task set
 * @param[in|out] u the total utilization
 * @param[in|out] r a random number generator
 * @param[out] debug stream to send debugging output, can be NULL.
 *
 * @return 0 if the task set is well formed and modified, less than
 * zero if the task set is poorly formed, greater than zero if the
 * task set could not be modified for other reasons
 */
int uunifast(task_set_t *ts, double u, gsl_rng *r, FILE *debug);

/**
 * Implementation of UUniFast "Biasing Effects in Schedulability",
 *   by Bini & Buttazo
 *
 * Caller interprets the meaning of utilization assignment.
 *
 * @param[in|out] ts the task set
 * @param[in|out] u the total utilization
 * @param[in|out] r a random number generator
 * @param[out] debug stream to send debugging output, can be NULL.
 * @param[in] callback user callback
 *
 * @return 0 if the task set is well formed and modified, less than
 * zero if the task set is poorly formed, greater than zero if the
 * task set could not be modified for other reasons
 */
int uunifast_cb(task_set_t *ts, double u, gsl_rng *r, FILE *debug,
		uu_updater callback);

#endif /* UUNIFAST_H */


#ifndef TASKSET_MOD_H
#define TASKSET_MOD_H
#include <unistd.h>
#include <gsl/gsl_rng.h>
#include "taskset.h"
#include "taskset-create.h"
#include "uunifast.h"

/**
 * Sets the number of threads per task based on the total number of
 * threads available.
 *
 * @param[in] r random source
 * @param[in] M the total number of threads to assign
 * @param[in] minm the smallest number of threads to assign to a
 * single task
 * @param[in] maxm the largest number of threads to assign to a single
 * task 
 *
 * @return non-zero upon success, zero upon out of memory.
 */
int tsm_dist_threads(gsl_rng *r, task_set_t* ts, int M, int minm, int maxm);

/**
 * Sets the periods of tasks based on their utilization calculated by UUnifast
 *
 * @param[in] r random source
 * @param[in] ts the task set
 * @param[in] u target utilization
 * @param[in|out] debug debuging output filehanlde
 *
 * @return non-zero upon success, zero upon out of memory.
 */
int tsm_uunifast_periods(gsl_rng *r, task_set_t* ts, double u, FILE *debug);

/**
 * Sets the deadlines of tasks based on their execution and period
 *
 * deadline = (in the distribution) [max(wcet, period / 2), max period]
 *
 * @param[in] r random source
 * @param[in] ts the task set
 * @param[in] u target utilization
 * @param[in|out] debug debuging output filehanlde
 *
 * @return non-zero upon success, zero upon out of memory.
 */
int tsm_set_deadlines(gsl_rng *r, task_set_t* ts, FILE *debug);

/**
 * Forces those tasks with convex WCET functions to be vacuously
 * concave 
 *
 * @param[in|out] ts the task set
 * @param[in|out] debug debuging output filehanlde
 *
 * @return non-zero upon success, zero upon out of memory.
 */
int tsm_force_concave(task_set_t *ts, FILE *debug);

#endif /* TASKSET_MOD_H */


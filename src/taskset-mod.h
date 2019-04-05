#ifndef TASKSET_MOD_H
#define TASKSET_MOD_H
#include <unistd.h>
#include <gsl/gsl_rng.h>
#include "taskset.h"
#include "taskset-create.h"

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
#endif /* TASKSET_MOD_H */


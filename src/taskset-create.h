#ifndef TASKSET_CREATE_H
#define TASKSET_CREATE_H
#include <gsl/gsl_rng.h>
#include "taskset.h"

/**
 * Adds n tasks with no parameters to the task set.
 *
 * n bare tasks are added to the set, with a period, deadline, and
 * number of threads equal to zero 
 *
 * @param[in] n the number of tasks to add
 *
 * @return non-zero upon success, zero upon out of memory.
 */
int tsc_bare_addn(task_set_t* ts, int n);

/**
 * Adds a variable number of tasks with to the task set, where the
 * total number of executable threads is equal to totalm.
 *
 * The number of threads assigned to a task is take from a uniform
 * distribution (determined by r) over [minm, maxm] inclusive.
 *
 * @param[in] ts the task set
 * @param[in] r a random source
 * @param[in] totalm the total number of threads
 * @param[in] minm, maxm the minimum and maximum number of threads
 *    assigned to one task
 *
 * @return number of tasks added 
 */
int tsc_add_by_thread_count(task_set_t* ts, gsl_rng *r, uint32_t totalm,
    uint32_t minm, uint32_t maxm);

/**
 * Sets the periods of all tasks given a random source and range
 *
 * @param[in|out] ts the task set
 * @param[in] r the random source
 * @param[in] minp the minimum period value (inclusive)
 * @param[in] maxp the maximum period value (inclusive)
 *
 * @return non-zero upon success
 */
int tsc_set_periods(task_set_t* ts, gsl_rng *r, uint32_t minp, uint32_t maxp);

/**
 * Sets the threads per job of all tasks given a random source and range
 *
 * @param[in|out] ts the task set
 * @param[in] r the random source
 * @param[in] minm the minimum threads per job (inclusive)
 * @param[in] maxm the maximum threads per job (inclusive)
 *
 * @return non-zero upon success
 */
int tsc_set_threads(task_set_t* ts, gsl_rng *r, uint32_t minm, uint32_t maxm);

/** 
 * Sets the deadlines of the tasks given a uniform random source,
 * limited by the WCET and half the period. 
 *
 * This function matches the distribution method described in 
 *   "Limited Preemption EDF Scheduling of Sporadic Task Systems"
 *   -- Bertogna & Baruah, 2010
 *
 * For each task in ts, the minimum deadline is the 
 *   mind = max(wcet, period / 2)
 *
 * The range is [mind, maxd] inclusive.
 * 
 * @param[in|out] ts the task set
 * @param[in] r the random source
 * @param[in] maxd the maximum deadline value
 *
 *
 * @return non-zero upon success, zero otherwise.
 */
int tsc_set_deadlines_min_halfp(task_set_t *ts, gsl_rng *r, uint32_t maxd);

/**
 * Sets the WCET of threads of tasks given a random source, for a
 * growth factor within a uniform distribution [minf, maxf]
 * 
 *
 * @param[in|out] ts the task set
 * @param[in] r the random source
 * @param[in] minf the minimum growth factor of a task (inclusive)
 * @param[in] maxf the maximum growth factor of a task (inclusive)
 *
 * @return non-zero upon success
 */
int tsc_set_wcet_gf(task_set_t* ts, gsl_rng *r, float minf, float maxf);


#endif /* TASKSET_CREATE_H */


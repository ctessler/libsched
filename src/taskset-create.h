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

#endif /* TASKSET_CREATE_H */


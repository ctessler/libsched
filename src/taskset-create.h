#ifndef TASKSET_CREATE_H
#define TASKSET_CREATE_H
#include <unistd.h>
#include <gsl/gsl_rng.h>
#include "taskset.h"

typedef struct {
	uint32_t gp_totalm;	/**< Total # of threads in the set */
	uint32_t gp_minm;	/**< Minimum # of threads per job */
	uint32_t gp_maxm;	/**< Maximim # of threads per job */
	uint32_t gp_minp;	/**< Minimum period */
	uint32_t gp_maxp;	/**< Maximum period */
	uint32_t gp_mind;	/**< Minimum relative deadline */
	uint32_t gp_maxd;	/**< Maximum relative deadline */
	uint32_t gp_wcet_scale;	/**< Scaling WCET maximum*/
	float_t gp_util;	/**< Task set utilization */
	float_t gp_minf;	/**< Minimum growth factor */
	float_t gp_maxf;	/**< Maximum growth factor */	
} gen_parms_t;

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
int tsc_add_by_thread_count(task_set_t* ts, gsl_rng *r, tint_t totalm,
    tint_t minm, tint_t maxm);

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
int tsc_set_periods(task_set_t* ts, gsl_rng *r, tint_t minp, tint_t maxp);

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
int tsc_set_threads(task_set_t* ts, gsl_rng *r, tint_t minm, tint_t maxm);

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
 * @param[in] mind lower bound on the deadline
 * @param[in] maxd the maximum deadline value
 *
 * @return non-zero upon success, zero otherwise.
 */
int tsc_set_deadlines_min_halfp(task_set_t *ts, gsl_rng *r, tint_t mind, tint_t maxd);

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

/**
 * Quiet gsl_env_setup()
 *
 * It's unclear why there isn't an option to gsl_env_setup() to avoid
 * spamming to stderr. Yet it INSISTS that a user providing an
 * environment variable *is* indeed an error.
 * 
 * Use this function if an environment variable is *NOT* an error
 */
void ges_stfu();

/**
 * Returns a tint_t from the distribution given by random source 
 * between min and max
 *
 * @param[in] r random source
 * @param[in] min imum value
 * @param[in] max imum value
 *
 * @return a number between min and max (inclusive)
 */
tint_t tsc_get_scaled(gsl_rng *r, tint_t min, tint_t max);

/**
 * Returns a double from the distribution given by random source 
 * between min and max
 *
 * @param[in] r random source
 * @param[in] min imum value
 * @param[in] max imum value
 *
 * @return a number between min and max (inclusive)
 */
double tsc_get_scaled_dbl(gsl_rng *r, double min, double max);

#endif /* TASKSET_CREATE_H */


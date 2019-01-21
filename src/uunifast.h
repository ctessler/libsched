#ifndef UUNIFAST_H
#define UUNIFAST_H
#include <stdio.h>
#include <gsl/gsl_rng.h>
#include "taskset.h"

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

#endif /* UUNIFAST_H */


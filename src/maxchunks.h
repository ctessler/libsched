#ifndef MAXCHUNKS_H
#define MAXCHUNKS_H
#include "taskset.h"
#include "taskset-deadlines.h"
/**
 * Implementation of Baruah's maximum non-preemptive chunk algorithm
 *
 * Modifies the chunk sizes for all of the tasks within the task set.
 *
 * @param[in|out] ts the task set
 *
 * @return 0 if the task set is well formed and feasible, less than
 * zero if the task set is poorly formed, greater than zero if the
 * task set is infeasible
 */
int max_chunks(task_set_t *ts);

/**
 * Non-preemptive check
 *
 * Verifies the task set could be scheduled by non-preemptive EDF,
 * after having max_chunks run upon it
 *
 * @param[in] ts the task set
 *
 * @return 0 if the task set is feasible, non-zero otherwise
 */
int max_chunks_nonp(task_set_t *ts);

#endif /* MAXCHUNKS_H */


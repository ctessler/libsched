#ifndef TPJ_H
#define TPJ_H
#include "taskset.h"
#include "ordl.h"

/**
 * Implementation of Tessler's Thread Per Job Algorithm
 *
 * Modifies the number of tasks in the task set as well as the number
 * of threads per job 
 *
 * @param[in|out] ts the task set
 *
 * @return 0 if the task set is well formed and feasible, less than
 * zero if the task set is poorly formed, greater than zero if the
 * task set is infeasible
 */
int tpj(task_set_t *ts);

#endif /* TPJ_H */


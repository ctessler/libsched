#ifndef DAG_TASK_TO_TASK_H
#define DAG_TASK_TO_TASK_H

#include "dag-task-set.h"
#include "taskset.h"

/**
 * Convert a dag task to a task
 *
 * @param[in] dt the dag task
 *
 * @return a single threaded task that must be task_free()'d
 */
task_t *dt_to_t(dtask_t *dt);


#endif /* DAG_TASK_TO_TASK_H */

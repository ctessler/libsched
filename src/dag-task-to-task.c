#include "dag-task-to-task.h"

/**
 * Convert a dag task to a task
 *
 * @param[in] dt the dag task
 *
 * @return a single threaded task that must be task_free()'d
 */
task_t *dt_to_t(dtask_t *dt) {
	task_t *t = task_alloc(dt->dt_period, dt->dt_deadline, 1);
	t->wcet(1) = dtask_workload(dt);

	return t;
}

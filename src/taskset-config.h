#ifndef TASKSET_CONFIG_H
#define TASKSET_CONFIG_H

#include <libconfig.h>
#include "taskset.h"

/**
 * Processes the parsed configuration file into a set of tasks.
 *
 * @param[in] cfg the in memory configuration file
 * @param[in|out] ts the task set resulting from processing cfg
 *
 * @return non-zero upon success, zero otherwise
 */
int ts_config_process(config_t *cfg, task_set_t *ts);

#endif /* TASKSET_CONFIG_H */

#ifndef DTASKSET_CONFIG_H
#define DTASKSET_CONFIG_H

#include <libconfig.h>
#include "dag-task-set.h"

/**
 * Processes the parsed configuration file into a set of tasks.
 *
 * @param[in] cfg the in memory configuration file
 * @param[in] dir directory containing the original configuration file
 * @param[in|out] dts the task set resulting from processing cfg
 *
 * @return non-zero upon success, zero otherwise
 */
int dts_config_process(config_t *cfg, char *dir, dtask_set_t *dts);

/**
 * Dump the task set into a configuration object
 *
 * cfg **must** have been config_init()'d before this call.
 *
 * @param[out] cfg the in memory configuration
 * @param[in] ts the task set
 *
 * @return non-zero upon success, zero otherwise
 */
int dts_config_dump(config_t *cfg, dtask_set_t *dts);

#endif /* TASKSET_CONFIG_H */

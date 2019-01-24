#ifndef TASKSET_CONFIG_H
#define TASKSET_CONFIG_H

#include <libconfig.h>
#include "taskset.h"
#include "taskset-create.h"

/**
 * Processes the parsed configuration file into a set of tasks.
 *
 * @param[in] cfg the in memory configuration file
 * @param[in|out] ts the task set resulting from processing cfg
 *
 * @return non-zero upon success, zero otherwise
 */
int ts_config_process(config_t *cfg, task_set_t *ts);

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
int ts_config_dump(config_t *cfg, task_set_t *ts);

/**
 * Reads the taskset generation configuration parameters
 *
 * @param[in] cfg the in memory configuartion
 * @param[out] parms the parameters read from cfg
 *
 * @return non-zero upon success, zero otherwise
 */
int ts_parm_process(config_t *cfg, gen_parms_t *parms);

/**
 * Dumps the taskset generation configuration parameters into a
 * configuration object.
 *
 * @param[out] cfg the in memory configuartion
 * @param[in] parms the generation parameters
 *
 * @return non-zero upon success, zero otherwise
 */
int ts_parm_dump(config_t *cfg, gen_parms_t *parms);

#endif /* TASKSET_CONFIG_H */

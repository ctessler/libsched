#ifndef DAG_TASK_H
#define DAG_TASK_H

#include <gvc.h>
#include "task.h"

#define DT_THREADS	"threads" 	/** Number of threads per node */
#define DT_OBJECT	"object"	/** Executable object */
#define DT_WCET_ONE	"wcetone"	/** WCET of ONE thread */
#define DT_FACTOR	"factor"	/** Growth factor of node */

typedef struct {
	Agraph_t *dt_graph;
} dtask_t;

/**
 * Allocates a DAG Task
 *
 * @param[in] name the name of the task
 *
 * @return the dag task upon success, NULL otherwise
 */
dtask_t *dtask_alloc(char* name);

/**
 * Releasse a DAG Task
 */
void dtask_free(dtask_t *task);

   
#endif /* TASK_H */

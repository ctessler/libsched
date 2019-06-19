#include "dag-task.h"

dtask_t *
dtask_alloc(char* name) {
	dtask_t *task = calloc(1, sizeof(dtask_t));
	task->dt_graph = agopen(name, Agdirected, NULL);

	/* Set defaults for the graph */
	agattr(task->dt_graph, AGNODE, "shape", "record");
	agattr(task->dt_graph, AGNODE, DT_THREADS, "0");
	agattr(task->dt_graph, AGNODE, DT_OBJECT, "0");
	agattr(task->dt_graph, AGNODE, DT_WCET_ONE, "0");
	agattr(task->dt_graph, AGNODE, DT_FACTOR, "0");
	
	return task;
}

void
dtask_free(dtask_t *task) {
	agclose(task->dt_graph);
	task->dt_graph = NULL;
	free(task);
}


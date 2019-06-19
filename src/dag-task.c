#include "dag-task.h"

static GVC_t *gvc = NULL;

static void dnode_to_agnode(dnode_t *src, Agnode_t *dst);
static void agnode_to_dnode(Agnode_t *src, dnode_t *dst);

/**
 * DAG TASK
 */

dtask_t *
dtask_alloc(char* name) {
	if (!gvc) {
		gvc = gvContext();
	}
	dtask_t *task = calloc(1, sizeof(dtask_t));
	strncpy(task->dt_name, name, DT_NAMELEN);
	task->dt_graph = agopen(name, Agdirected, NULL);

	/* Set defaults for the graph */
	agattr(task->dt_graph, AGNODE, "shape", "record");
	agattr(task->dt_graph, AGNODE, DT_THREADS, "0");
	agattr(task->dt_graph, AGNODE, DT_OBJECT, "0");
	agattr(task->dt_graph, AGNODE, DT_WCET_ONE, "0");
	agattr(task->dt_graph, AGNODE, DT_WCET, "0");
	agattr(task->dt_graph, AGNODE, DT_FACTOR, "0");

	return task;
}

void
dtask_free(dtask_t *task) {
	agclose(task->dt_graph);
	task->dt_graph = NULL;
	free(task);
}

dnode_t *
dtask_name_search(dtask_t *task, char *name) {
	Agnode_t *ag_node = agnode(task->dt_graph, name, FALSE);
	if (!ag_node) {
		return NULL;
	}
	dnode_t *node = dnode_alloc(name);
	agnode_to_dnode(ag_node, node);
	node->dn_task = task;
	node->dn_node = ag_node;
	node->dn_dirty = 0;

	return node;
}

int
dtask_insert(dtask_t *task, dnode_t *node) {
	/* Search for the node first */
	Agnode_t *ag_node = agnode(task->dt_graph, node->dn_name, FALSE);
	if (ag_node) {
		/* Exists */
		return 0;
	}
	ag_node = agnode(task->dt_graph, node->dn_name, TRUE);
	if (!ag_node) {
		/* Could not allocate */
		return 0;
	}
	/* Fill node values into ag_node */
	dnode_to_agnode(node, ag_node);

	/* Track last insertion */
	node->dn_node = ag_node;
	node->dn_task = task;
	node->dn_dirty = 0;

	return 1;
}

int
dtask_write(dtask_t *task, FILE *file) {
	gvLayout(gvc, task->dt_graph, "dot");
	gvRender(gvc, task->dt_graph, "dot", file);
	gvFreeLayout(gvc, task->dt_graph);
	return 1;
}

/**
 * DAG NODE
 */
dnode_t *
dnode_alloc(char* name) {
	dnode_t *node = calloc(1, sizeof(dnode_t));
	strncpy(node->dn_name, name, DT_NAMELEN);
}

void
dnode_free(dnode_t *node) {
	memset(node->dn_name, 0, DT_NAMELEN);
	free(node);
}


/**
 * Updates the Agnode from the dnode
 */
static void
dnode_to_agnode(dnode_t *src, Agnode_t *dst) {
	char buff[DT_NAMELEN];

	/* Set the attributes of the node */
	sprintf(buff, "%ld", src->dn_object);
	agset(dst, DT_OBJECT, buff);

	sprintf(buff, "%ld", src->dn_threads);
	agset(dst, DT_THREADS, buff);

	sprintf(buff, "%ld", src->dn_wcet_one);
	agset(dst, DT_WCET_ONE, buff);

	sprintf(buff, "%ld", src->dn_wcet);
	agset(dst, DT_WCET, buff);

	sprintf(buff, "%f", src->dn_factor);
	agset(dst, DT_FACTOR, buff);
}

/**
 * Fills in the dnode with values from the Agnode
 */
static void
agnode_to_dnode(Agnode_t *src, dnode_t *dst) {
	dst->dn_object = atoi(agget(src, DT_OBJECT));
	dst->dn_threads = atoi(agget(src, DT_THREADS));
	dst->dn_wcet_one = atoi(agget(src, DT_WCET_ONE));
	dst->dn_wcet = atoi(agget(src, DT_WCET));
	dst->dn_factor = atof(agget(src, DT_FACTOR));
}

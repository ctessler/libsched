#include "dag-task.h"

static GVC_t *gvc = NULL;

static void dnode_to_agnode(dnode_t *src, Agnode_t *dst);
static void agnode_to_dnode(Agnode_t *src, dnode_t *dst);
static void dedge_make_label(dedge_t *edge);

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
	task->dt_graph = agopen(name, Agstrictdirected, NULL);

	/* Set defaults for the graph */
	agattr(task->dt_graph, AGNODE, "shape", "rectangle");
	agattr(task->dt_graph, AGNODE, DT_THREADS, "0");
	agattr(task->dt_graph, AGNODE, DT_OBJECT, "0");
	agattr(task->dt_graph, AGNODE, DT_WCET_ONE, "0");
	agattr(task->dt_graph, AGNODE, DT_WCET, "0");
	agattr(task->dt_graph, AGNODE, DT_FACTOR, "0");
	agattr(task->dt_graph, AGNODE, "texlbl", "");

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
dtask_name_remove(dtask_t *task, char *name) {
	Agnode_t *ag_node = agnode(task->dt_graph, name, FALSE);
	if (!ag_node) {
		return 0;
	}
	agdelete(task->dt_graph, ag_node);

	return 1;
}

int
dtask_remove(dtask_t *task, dnode_t *node) {
	if (!dtask_name_remove(task, node->dn_name)) {
		return 0;
	}

	node->dn_task = NULL;
	node->dn_node = NULL;
	node->dn_dirty = 0;

	return 1;
}

int
dtask_insert_edge(dtask_t *task, dnode_t *src, dnode_t *dst) {
	char buff[DT_NAMELEN * 2 + 5];
	if (src->dn_dirty || dst->dn_dirty) {
		/* Nodes must be updated before an edge can be added */
		return 0;
	}
	if (src->dn_task != task || dst->dn_task != task) {
		/* Nodes must be in the task */
		return 0;
	}
	Agedge_t *edge = agedge(task->dt_graph, src->dn_node, dst->dn_node, NULL, FALSE);
	if (edge) {
		/* Edge is already present */
		return 0;
	}
	sprintf(buff, "%s -> %s", src->dn_name, dst->dn_name);
	edge = agedge(task->dt_graph, src->dn_node, dst->dn_node, buff, TRUE);
	if (!edge) {
		/* Could not add the edge */
		return 0;
	}
	return 1;
}

int
dtask_remove_edge(dtask_t *task, dnode_t *src, dnode_t *dst) {
	dnode_t *a = dtask_name_search(task, src->dn_name);
 	dnode_t *b = dtask_name_search(task, dst->dn_name);

	if (!a || !b) {
		return 0;
	}

	Agedge_t *edge = agedge(task->dt_graph, a->dn_node, b->dn_node, NULL, FALSE);
	free(a);
	free(b);
	if (!edge) {
		return 0;
	}

	agdelete(task->dt_graph, edge);
	return 1;
}

dedge_t *
dtask_search_edge(dtask_t *task, char *sname, char *dname) {
	dnode_t *a = dtask_name_search(task, sname);
	dnode_t *b = dtask_name_search(task, dname);

	if (!a || !b) {
		return 0;
	}

	Agedge_t *edge = agedge(task->dt_graph, a->dn_node, b->dn_node, NULL, FALSE);
	free(a);
	free(b);
	if (!edge) {
		return NULL;
	}

	dedge_t *e = dedge_alloc(agnameof(edge));
	dedge_set_src(e, sname);
	dedge_set_dst(e, dname);
	dedge_make_label(e);

	return e;
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

	return node;
}

void
dnode_free(dnode_t *node) {
	memset(node->dn_name, 0, DT_NAMELEN);
	free(node);
}

/**
 * Adds the LaTeX label to the node
 */
static void
dnode_make_label(dnode_t *node) {
	sprintf(node->dn_label, "${%s = \\langle o_%ld, %ld, %ld, %f \\rangle}$",
		node->dn_name, node->dn_object, node->dn_threads, node->dn_wcet,
		node->dn_factor);
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

	dnode_make_label(src);
	agset(dst, "texlbl", src->dn_label);
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
	dnode_make_label(dst);
	dst->dn_dirty = 0;
}

/**
 * Calculates the WCET
 */
static void
dnode_calc_wcet(dnode_t *node) {
	tint_t threads = node->dn_threads;
	if (threads == 0) {
		node->dn_wcet = 0;
	}
	tint_t wcet1 = node->dn_wcet_one;
	float_t factor = node->dn_factor;

	tint_t wcet = wcet1 + ceil((threads - 1) * wcet1 * factor);

	node->dn_wcet = wcet;
}

/**
 * Node getters and setters
 */
tint_t
dnode_get_object(dnode_t *node) {
	return node->dn_object;
}
void
dnode_set_object(dnode_t *node, tint_t obj) {
	if (node->dn_object == obj) {
		return;
	}
	node->dn_object = obj;
	node->dn_dirty = 1;
}
tint_t
dnode_get_threads(dnode_t *node) {
	return node->dn_threads;
}
void
dnode_set_threads(dnode_t *node, tint_t threads) {
	if (node->dn_threads == threads) {
		return;
	}
	node->dn_threads = threads;
	node->dn_dirty = 1;
}
tint_t
dnode_get_wcet_one(dnode_t *node) {
	return node->dn_wcet_one;
}
void
dnode_set_wcet_one(dnode_t *node, tint_t wcet_one) {
	if (node->dn_wcet_one == wcet_one) {
		return;
	}
	node->dn_wcet_one = wcet_one;
	node->dn_dirty = 1;
}
tint_t
dnode_get_wcet(dnode_t *node) {
	if (node->dn_dirty) {
		dnode_calc_wcet(node);
	}
	return node->dn_wcet;
}
float_t
dnode_get_factor(dnode_t *node) {
	return node->dn_factor;
}
void
dnode_set_factor(dnode_t *node, float_t factor) {
	if (node->dn_factor == factor) {
		return;
	}
	node->dn_factor = factor;
	node->dn_dirty = 1;
}


int
dnode_update(dnode_t *node) {
	/* Search for the node first */
	Agraph_t *ag_graph = node->dn_task->dt_graph;
	Agnode_t *ag_node = agnode(ag_graph, node->dn_name, FALSE);
	if (!ag_node) {
		/* Does not exist */
		return 0;
	}
	if (!node->dn_dirty) {
		/* Nothing dirty, nothing to do */
		return 1;
	}
	dnode_calc_wcet(node);
	dnode_to_agnode(node, ag_node);
	node->dn_dirty = 0;

	return 1;
}

/*
 * DAG Edge
 */
dedge_t*
dedge_alloc(char* name) {
	dedge_t *edge = calloc(1, sizeof(dedge_t));
	strncpy(edge->de_name, name, DT_NAMELEN);

	return edge;
}

void
dedge_free(dedge_t *edge) {
	free(edge);
}

void
dedge_set_src(dedge_t *edge, char *name) {
	strncpy(edge->de_sname, name, DT_NAMELEN);
}

void
dedge_set_dst(dedge_t *edge, char *name) {
	strncpy(edge->de_dname, name, DT_NAMELEN);
}

void
dedge_make_label(dedge_t *edge) {
	sprintf(edge->de_label, "%s -> %s", edge->de_sname, edge->de_dname);
}

#include "dag-task.h"
#include "dag-walk.h"
static GVC_t *gvc = NULL;

static void dnode_to_agnode(dnode_t *src, Agnode_t *dst);
static void agnode_to_dnode(Agnode_t *src, dnode_t *dst);
static void dedge_make_label(dedge_t *edge);
static void agedge_to_dedge(Agedge_t *src, dedge_t *dst);

static tint_t
fact(tint_t n) {
	tint_t rv = 1;
	for (tint_t i = n ; i > 1 ; --i) {
		rv *= i;
	}
	return rv;
}

/**
 * DAG TASK
 */
static void
dtask_source_workload(dtask_t *task) {
	Agnode_t *n;
	dnode_t *source, *node;
	tint_t workload = 0;
	for (n = agfstnode(task->dt_graph); n; n = agnxtnode(task->dt_graph, n)) {
		int indegree = agdegree(task->dt_graph, n, TRUE, FALSE);
		if (indegree == 0) {
			source = dnode_alloc(agnameof(n));
			agnode_to_dnode(n, source);
			source->dn_task = task;
			dnode_free(task->dt_source);
			task->dt_source = source;
		}
		tint_t wcet = atoi(agget(n, DT_WCET));
		workload += wcet;
	}
	task->dt_workload = workload;
}

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
	agattr(task->dt_graph, AGNODE, DT_MARKED, "0");
	agattr(task->dt_graph, AGNODE, DT_VISITED, "0");
	agattr(task->dt_graph, AGNODE, DT_DISTANCE, "0");	
	agattr(task->dt_graph, AGNODE, "texlbl", "");
	agattr(task->dt_graph, AGRAPH, DT_DEADLINE, "0");
	agattr(task->dt_graph, AGRAPH, DT_PERIOD, "0");	
	agattr(task->dt_graph, AGRAPH, DT_WORKLOAD, "0");
	agattr(task->dt_graph, AGRAPH, DT_CPATHLEN, "0");
	return task;
}

void
dtask_free(dtask_t *task) {
	if (task->dt_source) {
		dnode_free(task->dt_source);
	}
	if (task->dt_graph) {
		agclose(task->dt_graph);
		task->dt_graph = NULL;
	}
	free(task);
}

dtask_t *
dtask_copy(dtask_t *task) {
	dtask_t *ntask = NULL;
	if (!task) {
		return NULL;
	}
	FILE *tmp = tmpfile();
	if (!tmp) {
		return NULL;
	}
	if (!dtask_write(task, tmp)) {
		goto bail;
	}
	rewind(tmp);

	ntask = calloc(1, sizeof(dtask_t));
	strncpy(ntask->dt_name, task->dt_name, DT_NAMELEN);
	ntask->dt_graph = agread(tmp, NULL);
	if (!ntask->dt_graph) {
		goto bail;
	}
	
	ntask->dt_period = task->dt_period;
	ntask->dt_deadline = task->dt_deadline;
	ntask->dt_cpathlen = task->dt_cpathlen;
	ntask->dt_workload = task->dt_workload;

	fclose(tmp);
	return ntask;
bail:
	if (tmp) {
		fclose(tmp);
	}
	if (ntask) {
		dtask_free(ntask);
	}
	return NULL;
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
	node->dn_flags.dirty = 0;

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
	task->dt_flags.dirty = 1;
	/* Fill node values into ag_node */
	dnode_to_agnode(node, ag_node);

	/* Track last insertion */
	node->dn_node = ag_node;
	node->dn_task = task;
	node->dn_flags.dirty = 0;

	return 1;
}

int
dtask_name_remove(dtask_t *task, char *name) {
	Agnode_t *ag_node = agnode(task->dt_graph, name, FALSE);
	if (!ag_node) {
		return 0;
	}
	agdelete(task->dt_graph, ag_node);
	task->dt_flags.dirty = 1;

	return 1;
}

int
dtask_remove(dtask_t *task, dnode_t *node) {
	if (!dtask_name_remove(task, node->dn_name)) {
		return 0;
	}
	
	node->dn_task = NULL;
	node->dn_node = NULL;
	node->dn_flags.dirty = 0;

	return 1;
}

int
dtask_insert_edge(dtask_t *task, dnode_t *src, dnode_t *dst) {
	char buff[DT_NAMELEN * 2 + 5];
	if (src->dn_flags.dirty || dst->dn_flags.dirty) {
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
	task->dt_flags.dirty = 1;	
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
	task->dt_flags.dirty = 1;	
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
	agedge_to_dedge(edge, e);

	return e;
}

int
dtask_write(dtask_t *task, FILE *file) {
	#if 0 /* Don't do this, it'll be written to the file as a node */
	char buff[DT_NAMELEN * 2];
	sprintf(buff, "${L = %ld, C = %ld, D = %ld, P = %ld}$",
		dtask_cpathlen(task), dtask_workload(task), task->dt_deadline,
		task->dt_period);
	Agnode_t *n = agnode(task->dt_graph, "key", TRUE);
	agset(n, "label", buff);
	#endif
	
	gvLayout(gvc, task->dt_graph, "dot");
	gvRender(gvc, task->dt_graph, "dot", file);
	gvFreeLayout(gvc, task->dt_graph);

	#if 0
	agdelete(task->dt_graph, n);
	#endif
	return 1;
}

dtask_t *
dtask_read(FILE *file) {
	if (!gvc) {
		gvc = gvContext();
	}
	dtask_t *task = calloc(1, sizeof(dtask_t));
	task->dt_graph = agread(file, NULL);
	if (!task->dt_graph) {
		goto bail;
	}
	
	sprintf(task->dt_name, "%s", agnameof(task->dt_graph));
	task->dt_period = atoi(agget(task->dt_graph, DT_PERIOD));
	task->dt_deadline = atoi(agget(task->dt_graph, DT_DEADLINE));
	task->dt_cpathlen = atoi(agget(task->dt_graph, DT_CPATHLEN));
	task->dt_workload = atoi(agget(task->dt_graph, DT_WORKLOAD));
	dtask_source_workload(task);
	
	return task;
bail:
	if (task) {
		dtask_free(task);
	}
	return NULL;
}

dtask_t *
dtask_read_path(char* path) {
	dtask_t *task = NULL;
	FILE *file = fopen(path, "r");
	if (!file) {
		goto bail;
	}
	task = dtask_read(file);
	fclose(file);
bail:
	return task;
}

/**
 * Source must be valid before calling this function
 */
static void
dtask_find_cpathlen(dtask_t *task) {
	dtask_unmark(task);
	dnode_t **sorted = dag_maxd(task->dt_source);
	int sink = agnnodes(task->dt_graph) - 1;
	task->dt_cpathlen = sorted[sink]->dn_distance;

	for (int i=0; sorted[i]; i++) {
		dnode_free(sorted[i]);
	}
}

int
dtask_update(dtask_t *task) {
	char buff[DT_NAMELEN];	
	if (task->dt_flags.dirty) {
		dtask_source_workload(task);
	}
	dtask_find_cpathlen(task);

	/* Update the graph */
	sprintf(buff, "%ld", task->dt_period);
	agset(task->dt_graph, DT_PERIOD, buff);
	sprintf(buff, "%ld", task->dt_deadline);
	agset(task->dt_graph, DT_DEADLINE, buff);
	sprintf(buff, "%ld", task->dt_workload);
	agset(task->dt_graph, DT_WORKLOAD, buff);
	sprintf(buff, "%ld", task->dt_cpathlen);
	agset(task->dt_graph, DT_CPATHLEN, buff);

	
	return 1;
}


dnode_t *
dtask_source(dtask_t *task) {
	dtask_update(task);

	dnode_t *source = NULL;
	if (task->dt_source) {
		source = dtask_name_search(task, task->dt_source->dn_name);
	}
	
	return source;
}

tint_t
dtask_cpathlen(dtask_t* task) {
	dtask_update(task);

	return task->dt_cpathlen;
}

tint_t
dtask_workload(dtask_t* task) {
	dtask_update(task);

	return task->dt_workload;
}

void
dtask_unmark(dtask_t *task) {
	char buff[DT_NAMELEN];
	Agnode_t *n;
	for (n = agfstnode(task->dt_graph); n; n = agnxtnode(task->dt_graph, n)) {
		agset(n, DT_VISITED, "0");
		agset(n, DT_MARKED, "0");		
	}
	if (task->dt_source) {
		sprintf(buff, "%s", agnameof(task->dt_source->dn_node));
		dnode_free(task->dt_source);
		task->dt_source = dtask_name_search(task, buff);
	}
}

dnode_t *
dtask_next_node(dtask_t *task, dnode_t *node) {
	Agnode_t *agnext;
	dnode_t *next;
	if (!task) {
		return NULL;
	}
	if (node) {
		agnext = agnxtnode(task->dt_graph, node->dn_node);
	} else {
		agnext = agfstnode(task->dt_graph);
	}
	if (!agnext) {
		return NULL;
	}
	next = dnode_alloc(agnameof(agnext));
	agnode_to_dnode(agnext, next);
	next->dn_task = task;
	return next;
}

int
dtask_count_cand(dtask_t *task) {
	Agnode_t *agnext;

	int max = agnnodes(task->dt_graph);
	int *obj = calloc(max, sizeof(max));

	for (agnext = agfstnode(task->dt_graph); agnext;
	     agnext = agnxtnode(task->dt_graph, agnext)) {
		dnode_t* node = dnode_alloc(agnameof(agnext));
		agnode_to_dnode(agnext, node);
		int idx = node->dn_object;
		obj[idx] += 1;
		dnode_free(node);
	}
	int total = 0;
	for (int i=0; i < max; i++) {
		/*
		 * Combinations per object:
		 * / n \      k!
		 * |   | = -------
		 * \ k /   2! * (k - 2)!
		 *
		 */
		if (obj[i] <= 1) {
			continue;
		}
		int count = obj[i];
		int numer = fact(count);
		tint_t denom = 2 * fact(count - 2);
		total += numer / denom;
	}
	free(obj);
	return total;
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
	if (!node) {
		return;
	}
	memset(node->dn_name, 0, DT_NAMELEN);
	free(node);
}

/**
 * Adds the LaTeX label to the node
 */
static void
dnode_make_label(dnode_t *node) {
	sprintf(node->dn_label, "${d:%ld, %s = \\langle o_%ld, c_1:%ld, c(%ld):%ld, "
		"F:%0.2f \\rangle}$",
		node->dn_distance,
		node->dn_name, node->dn_object, 
		dnode_get_wcet_one(node), node->dn_threads, dnode_get_wcet(node),
		node->dn_factor);
}

/**
 * Updates the Agnode from the dnode
 */
static void
dnode_to_agnode(dnode_t *src, Agnode_t *dst) {
	char buff[DT_NAMELEN];

	/* Set the attributes of the node */
	sprintf(buff, "%ld", dnode_get_object(src));
	sprintf(buff, "%ld", src->dn_object);
	agset(dst, DT_OBJECT, buff);

	sprintf(buff, "%ld", dnode_get_threads(src));
	sprintf(buff, "%ld", src->dn_threads);	
	agset(dst, DT_THREADS, buff);

	sprintf(buff, "%ld", dnode_get_wcet_one(src));
	sprintf(buff, "%ld", src->dn_wcet_one);
	agset(dst, DT_WCET_ONE, buff);

	sprintf(buff, "%ld", dnode_get_wcet(src));
	sprintf(buff, "%ld", src->dn_wcet);
	agset(dst, DT_WCET, buff);

	sprintf(buff, "%f", dnode_get_factor(src));
	sprintf(buff, "%f", src->dn_factor);
	agset(dst, DT_FACTOR, buff);

	sprintf(buff, "%ld", src->dn_distance);
	agset(dst, DT_DISTANCE, buff);

	if (src->dn_flags.visited) {
		agset(dst, DT_VISITED, "1");
	}
	if (src->dn_flags.marked) {
		agset(dst, DT_MARKED, "1");
	}


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
	dst->dn_flags.dirty = 0;
	dst->dn_flags.marked = atoi(agget(src, DT_MARKED));	
	dst->dn_flags.visited = atoi(agget(src, DT_VISITED));
	dst->dn_node = src;
}

/**
 * Calculates the WCET
 */
static void
dnode_calc_wcet(dnode_t *node) {
	tint_t threads = node->dn_threads;
	if (threads == 0) {
		node->dn_wcet = 0;
		return;
	}
	if (node->dn_wcet_one == 0) {
		node->dn_wcet = 0;
		return;
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
	node->dn_flags.dirty = 1;
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
	node->dn_flags.dirty = 1;
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
	node->dn_flags.dirty = 1;
}
tint_t
dnode_get_wcet(dnode_t *node) {
	if (node->dn_flags.dirty) {
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
	node->dn_flags.dirty = 1;
}


int
dnode_update(dnode_t *node) {
	/* Search for the node first */
	if (!node->dn_task) {
		/* Cannot update a node that's not in a task */
		return 0;
	}
	Agraph_t *ag_graph = node->dn_task->dt_graph;
	Agnode_t *ag_node = agnode(ag_graph, node->dn_name, FALSE);
	if (!ag_node) {
		/* Does not exist */
		return 0;
	}
	if (!node->dn_flags.dirty) {
		/* Nothing dirty, nothing to do */
		return 1;
	}
	dnode_calc_wcet(node);
	dnode_to_agnode(node, ag_node);
	node->dn_flags.dirty = 0;

	return 1;
}

int
dnode_indegree(dnode_t *node) {
	if (!node) {
		return -1;
	}
	if (!node->dn_task) {
		return -1;
	}
	Agraph_t *g = node->dn_task->dt_graph;
	if (!g) {
		return -1;
	}
	Agnode_t *n = node->dn_node;
	if (!n) {
		return -1;
	}
	if (node->dn_flags.dirty) {
		dnode_update(node);
		n = node->dn_node;		
	}
	return agdegree(g, n, TRUE, FALSE);
}

int
dnode_outdegree(dnode_t *node) {
	if (!node) {
		return -1;
	}
	if (!node->dn_task) {
		return -1;
	}
	Agraph_t *g = node->dn_task->dt_graph;
	if (!g) {
		return -1;
	}
	Agnode_t *n = node->dn_node;
	if (!n) {
		return -1;
	}
	if (node->dn_flags.dirty) {
		dnode_update(node);
		n = node->dn_node;		
	}
	return agdegree(g, n, FALSE, TRUE);
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

static void
agedge_to_dedge(Agedge_t *src, dedge_t *dst) {
	snprintf(dst->de_name, DT_NAMELEN, "%s", agnameof(src));
	Agnode_t *a = agtail(src); /* These are backwards ... */
	Agnode_t *b = aghead(src); /* ... I don't know why */
	dedge_set_src(dst, agnameof(a));
	dedge_set_dst(dst, agnameof(b));
	dst->de_edge = src;
	dst->de_graph = agraphof(src); 
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

dedge_t*
dedge_out_first(dnode_t *node) {
	if (!node->dn_node) {
		return NULL;
	}
	Agedge_t *edge = agfstout(node->dn_task->dt_graph, node->dn_node);
	if (!edge) {
		return NULL;
	}
	dedge_t *e = dedge_alloc(agnameof(edge));
	agedge_to_dedge(edge, e);

	return e;
}
dedge_t*
dedge_out_first_name(dtask_t *task, char *name) {
	dnode_t *node = dtask_name_search(task, name);
	if (!node) {
		return NULL;
	}
	return dedge_out_first(node);

}
dedge_t*
dedge_out_next(dedge_t *edge) {
	Agedge_t *next = agnxtout(edge->de_graph, edge->de_edge);
	if (!next) {
		return NULL;
	}
	dedge_t *e = dedge_alloc("tmp");
	agedge_to_dedge(next, e);

	return e;
}

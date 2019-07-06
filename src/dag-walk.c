#include "dag-walk.h"

typedef struct {
	dnode_t**	id_list;
	int		id_count;
	int		id_cur;
} topo_data_t;

static ddo_t
topological_pre(dnode_t* node, void *userd) {
	if (node->dn_flags.visited) {
		return DFS_SKIP;
	}
	node->dn_flags.dirty = 1;
	node->dn_flags.visited = 1;
	dnode_update(node);

	return DFS_GOOD;	
}

static ddo_t
topological_post(dnode_t* node, void *userd) {
	topo_data_t *data = userd;
	dnode_t *copy = dtask_name_search(node->dn_task, node->dn_name);

	data->id_list[data->id_cur] = copy;
	data->id_cur--;
	return DFS_GOOD;	
}

dnode_t **
dag_topological(dnode_t* node) {
	topo_data_t userd;
	userd.id_count = agnnodes(node->dn_task->dt_graph);
	userd.id_list = calloc(userd.id_count + 1, sizeof(dnode_t*));
	userd.id_cur = userd.id_count - 1;

	int rv = ddfs(node, topological_pre, NULL, topological_post, &userd);
	dtask_unmark(node->dn_task);

	return userd.id_list;
}

dnode_t **
dag_maxd(dnode_t *node) {
	dtask_t *task = node->dn_task;
	char buff[DT_NAMELEN];
	dnode_t **topo = dag_topological(node);
	Agraph_t *g = task->dt_graph;

	for (int i=0; topo[i]; i++) {
		Agnode_t *n = topo[i]->dn_node;
		Agedge_t *e;
		tint_t maxd = 0;
		for (e = agfstin(g,n); e; e = agnxtin(g,e)) {
			Agnode_t *a = agtail(e); /* These are backwards ... */
			Agnode_t *b = aghead(e); /* ... I don't know why */
			tint_t prec_d = atoi(agget(a, DT_DISTANCE));
			if (prec_d > maxd) {
				maxd = prec_d;
			}
		}
		/* The distance of each node is the maximum completion time */
		topo[i]->dn_distance = dnode_get_wcet(topo[i]) + maxd;

		/* Update the task */
		dnode_t *this = dtask_name_search(task, topo[i]->dn_name);
		this->dn_distance = topo[i]->dn_distance;
		this->dn_flags.dirty = 1;
		dnode_update(this);
		dnode_free(this);
	}
	return topo;
}

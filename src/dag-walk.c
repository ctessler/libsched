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

	return userd.id_list;
}



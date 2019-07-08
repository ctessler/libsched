#include "dag-collapse.h"

extern void agnode_to_dnode(Agnode_t *src, dnode_t *dst);

static char **
ingress_nodes(dnode_t *node) {

}

static tint_t
fact(tint_t n) {
	tint_t rv = 1;
	for (tint_t i = n ; i > 1 ; --i) {
		rv *= i;
	}
	return rv;
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


int
dag_can_collapse(dnode_t *a, dnode_t *b) {
	if (dnode_get_object(a) != dnode_get_object(b)) {
		return 0;
	}
	int rv = 0;

	dnode_t *cp_a = dnode_copy(a);
	dnode_t *cp_b = dnode_copy(b);

	if (dag_pathlen(a, b) > 2) {
		goto bail;
	}
	if (dag_pathlen(b, a) > 2) {
		goto bail;
	}

	rv = 1;
bail:
	dnode_free(cp_a);
	dnode_free(cp_b);	
	return rv;
}



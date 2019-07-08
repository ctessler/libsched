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
dtask_can_collapse(dnode_t *a, dnode_t *b) {
	int count;
	int rv = 0;
	dnl_t *pred_a = dnl_preds(a);
	dnl_t *succ_a = dnl_succs(a);
	dnl_t *pred_b = dnl_preds(b);
	dnl_t *succ_b = dnl_succs(b);

	/*
	 * To avoid creating a loop
	 *
	 * No path between a and b (greater than length 1) is permitted.
	 */

	count = dnl_sharedc(pred_b, succ_a);
	if (count > 0) {
		goto bail;
	}

 bail:
	dnl_clear(pred_a);
	dnl_clear(succ_a);
	dnl_clear(pred_b);
	dnl_clear(succ_b);

	free(pred_a);
	free(succ_a);
	free(pred_b);
	free(succ_b);

	return rv;
}



#include "dag-collapse.h"

extern void agnode_to_dnode(Agnode_t *src, dnode_t *dst);

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

	int max = dtask_max_object(task) + 1;
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


int
dag_collapse(dnode_t* a, dnode_t* b) {
	char buff[DT_NAMELEN];
	/* Get the list of preds/succs */
	dnl_t *preds = dnl_preds(a);
	dnl_t *preds_b = dnl_preds(b);
	dnl_t *succs = dnl_succs(a);
	dnl_t *succs_b = dnl_succs(b);

	dnl_append(preds, preds_b);
	dnl_append(succs, succs_b);	


	/* Create the new node */
	snprintf(buff, DT_NAMELEN, "%s,%s", a->dn_name, b->dn_name);
	dnode_t *n = dnode_copy(a);
	sprintf(n->dn_name, "%s", buff);
	dnode_set_threads(n, dnode_get_threads(a) + dnode_get_threads(b));

	/* Insert the new node into the task */
	dtask_t *task = a->dn_task;
	dtask_insert(task, n);

	/* Insert edges from the predecessors */
	dnl_elem_t *cursor;
	dnl_foreach(preds, cursor) {
		dtask_insert_edge(task, cursor->dnl_node, n);
	}

	/* Insert edges to the successors */
	dnl_foreach(succs, cursor) {
		dtask_insert_edge(task, n, cursor->dnl_node);
	}

	/* Remove the old nodes */
	dtask_remove(task, a);
	dtask_remove(task, b);

	/* Release the node lists */
	dnl_clear(preds); free(preds);
	dnl_clear(preds_b); free(preds_b);	
	dnl_clear(succs); free(succs);
	dnl_clear(succs_b); free(succs_b);

	/* Update the task */
	dtask_update(task);

	return 1;
}

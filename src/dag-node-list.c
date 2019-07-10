#include "dag-node-list.h"

void agnode_to_dnode(Agnode_t *src, dnode_t *dst);

dnl_t*
dnl_alloc() {
	dnl_t *h = calloc(1, sizeof(dnl_t));
	return h;
}
void
dnl_free(dnl_t *head) {
	dnl_init(head);
	free(head);
}


dnl_elem_t*
dnle_alloc(dnode_t *node) {
	dnl_elem_t *e = calloc(sizeof(dnl_elem_t), 1);
	e->dnl_node = dnode_copy(node);
	return e;
}

dnl_elem_t*
dnle_free(dnl_elem_t* e) {
	/* If the node is on the list, it has been copied here */
	dnode_free(e->dnl_node);
	e->dnl_node = NULL;
	free(e);
}

dnl_elem_t*
dnle_copy(dnl_elem_t *e) {
	dnl_elem_t *cp = dnle_alloc(e->dnl_node);
	return cp;
}


int
dnl_insert(dnl_t* head, dnl_elem_t *elem) {
	if (dnl_empty(head)) {
		dnl_insert_head(head, elem);
		return 1;
	}
	/* c - cursor in the list */
	dnl_elem_t *first = LIST_FIRST(head);

	/* Insertion order is not important, prepend */
	dnl_insert_before(first, elem);
	return 1;
}

dnl_elem_t *
dnl_find(dnl_t* head, char *name) {
	if (dnl_empty(head)) {
		return NULL;
	}

	/* c - cursor in the list */
	dnl_elem_t *c = LIST_FIRST(head);
	while (c && !dnode_has_name(c->dnl_node, name)) {
		c = dnl_next(c);
	}
	return c;
}

void
dnl_clear(dnl_t *head) {
	dnl_elem_t *o_cursor, *tmp;

	for (o_cursor = dnl_first(head); o_cursor;) {
		tmp = dnl_next(o_cursor);
		dnl_remove(o_cursor);
		dnle_free(o_cursor);
		o_cursor = tmp;
	}
	dnl_init(head);
}

dnl_t *
dnl_preds(dnode_t *node) {
	dnl_t *h = calloc(1, sizeof(dnl_t));
	dnl_init(h);

	Agraph_t *g = node->dn_task->dt_graph;
	Agnode_t *n = node->dn_node;
	Agedge_t *e;

	for (e = agfstin(g,n); e; e = agnxtin(g,e)) {
		Agnode_t *a = agtail(e); /* a is the predecessor */
		Agnode_t *b = aghead(e); /* b is *this* node */

		dnode_t *pred = dnode_alloc(agnameof(a));
		agnode_to_dnode(a, pred);
		pred->dn_task = node->dn_task;
		dnl_elem_t *elem = dnle_alloc(pred);
		dnode_free(pred);
		dnl_insert_head(h, elem);
	}
	
	return h;
}

dnl_t *
dnl_succs(dnode_t *node) {
	dnl_t *h = calloc(1, sizeof(dnl_t));
	dnl_init(h);

	Agraph_t *g = node->dn_task->dt_graph;
	Agnode_t *n = node->dn_node;
	Agedge_t *e;

	for (e = agfstout(g,n); e; e = agnxtout(g,e)) {
		Agnode_t *a = agtail(e); /* a is *this* node */
		Agnode_t *b = aghead(e); /* b is the successor */

		dnode_t *succ = dnode_alloc(agnameof(b));
		agnode_to_dnode(b, succ);
		succ->dn_task = node->dn_task;		
		dnl_elem_t *elem = dnle_alloc(succ);
		dnode_free(succ);
		dnl_insert_head(h, elem);
	}
	
	return h;
}

dnl_t *
dnl_by_obj(dtask_t *task, tint_t object) {
	dnl_t *h = calloc(1, sizeof(dnl_t));
	dnl_init(h);

	dnode_t *n = NULL, *prev = NULL;
	while (n = dtask_next_node(task, prev)) {
		if (prev) {
			dnode_free(prev);
		}
		prev = n;
		if (dnode_get_object(n) != object) {
			continue;
		}
		dnl_elem_t *elem = dnle_alloc(n);
		dnl_insert_head(h, elem);
	}
	
	return h;
}


int
dnl_sharedc(dnl_t *a, dnl_t *b) {
	dnl_elem_t *a_cursor, *b_cursor;
	int count = 0;
	
	dnl_foreach(a, a_cursor) {
		char *name = a_cursor->dnl_node->dn_name;
		dnl_foreach(b, b_cursor) {
			if (dnode_has_name(b_cursor->dnl_node, name)) {
				count++;
			}
		}
	}
	return count;
}

int
dnl_append(dnl_t* orig, dnl_t* nlist) {
	dnl_elem_t *cursor;
	int count = 0;
	dnl_foreach(nlist, cursor) {
		char *name = cursor->dnl_node->dn_name;
		dnl_elem_t *inlist = dnl_find(orig, name);
		if (inlist) {
			/* Already in the predecessor list */
			continue;
		}
		dnl_elem_t *cp = dnle_copy(inlist);
		dnl_insert_head(orig, cp);
		count++;
	}
	return count;
}

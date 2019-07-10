#include "dag-candidate.h"

void agnode_to_dnode(Agnode_t *src, dnode_t *dst);

cand_list_t*
cand_list_alloc() {
	cand_list_t *head = calloc(1, sizeof(cand_list_t));
	return head;
}

cand_list_t*
cand_list_free(cand_list_t *head) {
	cand_list_empty(head);
	free(head);
}

void
cand_list_clear(cand_list_t *head) {
	cand_t *o_cursor, *tmp;

	for (o_cursor = cand_first(head); o_cursor;) {
		tmp = cand_next(o_cursor);
		cand_remove(o_cursor);
		cand_free(o_cursor);
		o_cursor = tmp;
	}
	cand_list_init(head);
}

void
cand_list_destroy(cand_list_t *head) {
	if (!head) {
		return;
	}
	cand_list_clear(head);
	cand_list_free(head);
}


cand_t*
cand_find(cand_list_t* head, char *name) {
	if (cand_list_empty(head)) {
		return NULL;
	}

	/* c - cursor in the list */
	cand_t *c = cand_first(head);
	while (c && strcmp(cand_name(c), name) != 0) {
		c = cand_next(c);
	}
	return c;
}


cand_t*
cand_alloc() {
	cand_t *c = calloc(1, sizeof(cand_t));

	return c;
}

void
cand_free(cand_t *c) {
	if (c->c_a) {
		dnode_free(c->c_a);
	}
	if (c->c_b) {
		dnode_free(c->c_b);
	}
	c->c_a = NULL;
	c->c_b = NULL;
	free(c);
}

cand_t*
cand_copy(cand_t *orig) {
	cand_t *new = cand_alloc();
	if (orig->c_a) {
		new->c_a = dnode_copy(orig->c_a);
	}
	if (orig->c_b) {
		new->c_b = dnode_copy(orig->c_b);
	}
	new->c_delta_c = orig->c_delta_c;
	new->c_delta_l = orig->c_delta_l;

	return new;
}

void
cand_set_nodes(cand_t *cand, dnode_t *a, dnode_t *b) {
	if (cand->c_a) {
		dnode_free(cand->c_a);
	}
	if (cand->c_b) {
		dnode_free(cand->c_b);
	}
	cand->c_a = NULL;
	cand->c_b = NULL;
	if (a) {
		cand->c_a = dnode_copy(a);
	}
	if (b) {
		cand->c_b = dnode_copy(b);
	}
}


char*
cand_name(cand_t *c) {
	static char buff[DT_NAMELEN];
	sprintf(buff, "%s,%s", c->c_a->dn_name, c->c_b->dn_name);

	return buff;
}



static dnode_t *
find_first_by_obj(dtask_t *task, tint_t object) {
	Agnode_t *agnode = agfstnode(task->dt_graph);
	dnode_t *next = dnode_alloc(agnameof(agnode));
	agnode_to_dnode(agnode, next);
	while (next->dn_object != object) {
		dnode_free(next);
		agnode = agnxtnode(task->dt_graph, agnode);
		if (!agnode) {
			return NULL;
		}
		next = dnode_alloc(agnameof(agnode));
		agnode_to_dnode(agnode, next);
	}
	return next;
}


static dnode_t *
find_next_by_obj(dtask_t* task, dnode_t *node) {
	Agnode_t *agnode = node->dn_node;

	while (agnode = agnxtnode(task->dt_graph, agnode)) {
		dnode_t *next = dnode_alloc(agnameof(agnode));
		agnode_to_dnode(agnode, next);
		if (next->dn_object == node->dn_object) {
			return next;
		}
		dnode_free(next);
	}
	return NULL;
}

static cand_t*
task_cand_first(dtask_t *task) {
	cand_t *cand = cand_alloc();
	tint_t object = 0;
	dnode_t *a = find_first_by_obj(task, object);
	dnode_t *b = NULL;
	if (a) {
		b = find_next_by_obj(task, a);
	}
	if (b) {
		/* Have a candidate */
		goto done;
	}
	tint_t max = dtask_max_object(task);
	while (!b) {
		dnode_free(a);
		a = NULL;
		object++;

		if (object > max) {
			goto done;
		}
		a = find_first_by_obj(task, object);
		if (a) {
			b = find_next_by_obj(task, a);
		}
	}
done:
	cand_set_nodes(cand, a, b);
	dnode_free(a);
	dnode_free(b);
	return cand;
}

cand_t*
task_cand_next(dtask_t *task, cand_t *cur) {
	dnode_t *a = NULL, *b = NULL, *next = NULL;
	cand_t *orig = NULL; 
	
	if (cur == NULL) {
		cur = task_cand_first(task);
		return cur;
	}
	//orig = cand_copy(cur);
	a = dnode_copy(cur->c_a);
	b = dnode_copy(cur->c_b);
	next = find_next_by_obj(task, cur->c_b);
	if (next) {
		/* There is another node to collapse with with the
		   same object */ 
		cand_set_nodes(cur, a, next);
		goto done;
	}
	/* Need to go to the next object */
	cand_free(cur);
	cur = NULL;
	int max = dtask_max_object(task);
	tint_t object = dnode_get_object(a);
	do {
		dnode_free(a); a = NULL;
		dnode_free(b); b = NULL;
		if (++object > max) {
			goto done;
		}
		a = find_first_by_obj(task, object);
		if (a) {
			b = find_next_by_obj(task, a);
		}
	} while (b == NULL);
	cur = cand_alloc();
	cand_set_nodes(cur, a, b);
		
done:
	dnode_free(a);
	dnode_free(b);
	dnode_free(next);
	if (cur->c_a == NULL) {
		0x0;
	}
	if (cur->c_b == NULL) {
		0x0;
	}

	//cand_free(orig);
	return cur;
}

cand_list_t*
corder_arb(dtask_t *task) {
	cand_list_t *list = cand_list_alloc();
	cand_t *next = NULL;

	while(next = task_cand_next(task, next)) {
		cand_t *copy = cand_copy(next);
		cand_insert_head(list, copy);
	}

	return list;
}



cand_list_t* corder_maxb(dtask_t *dts) {
	return NULL;
}
cand_list_t* corder_minp(dtask_t *dts) {
	return NULL;
}

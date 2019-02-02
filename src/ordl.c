#include "ordl.h"

or_elem_t*
oe_alloc() {
	or_elem_t *e = calloc(sizeof(or_elem_t), 1);
	e->oe_tasks = ts_alloc();

	return e;
}

or_elem_t*
oe_free(or_elem_t* e) {
	ts_free(e->oe_tasks);
	e->oe_tasks = NULL;
	free(e);
}


int
ordl_insert(ordl_t* head, or_elem_t *elem) {
	if (ordl_empty(head)) {
		ordl_insert_head(head, elem);
		return 1;
	}
	/* c - cursor in the list */
	or_elem_t *c = LIST_FIRST(head);
	/* The goal is to insert before c */
	while (elem->oe_deadline >= c->oe_deadline) {
		if (!ordl_next(c)) {
			/* c is the end of the list */
			ordl_insert_after(c, elem);
			return 1;
		}
		c = ordl_next(c);
	}
	ordl_insert_before(c, elem);
	return 1;
}

or_elem_t *
ordl_find(ordl_t* head, uint32_t deadline) {
	if (ordl_empty(head)) {
		return NULL;
	}

	/* c - cursor in the list */
	or_elem_t *c = LIST_FIRST(head);
	while (c && c->oe_deadline != deadline) {
		c = ordl_next(c);
	}
	return c;
}

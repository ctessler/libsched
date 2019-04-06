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
ordl_find(ordl_t* head, tint_t deadline) {
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

int
ordl_rem_deadlines_task(ordl_t *head, task_t *task) {
	or_elem_t *cursor, *next;
	int count = 0;

	for (cursor = ordl_first(head); cursor; cursor = next) {
		next = ordl_next(cursor);
		task_set_t *ts = cursor->oe_tasks;
		task_link_t* cookie = ts_find(ts, task);

		if (!cookie) {
			/* Task doesn't have this deadline */
			continue;
		}

		/* Task is in the deadline */
		ts_rem(ts, cookie);
		count++;
	}

	return count;
}

void
ordl_clear(ordl_t *head) {
	or_elem_t *o_cursor, *tmp;

	for (o_cursor = ordl_first(head); o_cursor;) {
		tmp = ordl_next(o_cursor);
		ordl_remove(o_cursor);
		oe_free(o_cursor);
		o_cursor = tmp;
	}
	ordl_init(head);
}

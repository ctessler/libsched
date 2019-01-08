#include "ordl.h"

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

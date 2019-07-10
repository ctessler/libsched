#include "string-list.h"

sl_elem_t*
sle_alloc() {
	sl_elem_t *e = calloc(sizeof(sl_elem_t), 1);
	return e;
}

sl_elem_t*
sle_free(sl_elem_t* e) {
	if (e->sl_str) {
		free(e->sl_str);
		e->sl_str = NULL;
	}
	free(e);
}

void
sle_set(sl_elem_t *e, char *str) {
	e->sl_str = strdup(str);
}

sl_elem_t*
sle_copy(sl_elem_t *e) {
	sl_elem_t *cp = sle_alloc();
	sle_set(cp, e->sl_str);
	return cp;
}


int
sl_insert(sl_t* head, sl_elem_t *elem) {
	if (sl_empty(head)) {
		sl_insert_head(head, elem);
		return 1;
	}
	/* c - cursor in the list */
	sl_elem_t *first = LIST_FIRST(head);

	/* Insertion order is not important, prepend */
	sl_insert_before(first, elem);
	return 1;
}

sl_elem_t *
sl_find(sl_t* head, char *name) {
	if (sl_empty(head)) {
		return NULL;
	}

	/* c - cursor in the list */
	sl_elem_t *c = LIST_FIRST(head);
	while (c && strcmp(c->sl_str, name) != 0) {
		c = sl_next(c);
	}
	return c;
}

void
sl_clear(sl_t *head) {
	sl_elem_t *o_cursor, *tmp;

	for (o_cursor = sl_first(head); o_cursor;) {
		tmp = sl_next(o_cursor);
		sl_remove(o_cursor);
		sle_free(o_cursor);
		o_cursor = tmp;
	}
	sl_init(head);
}

int
sl_append(sl_t* orig, sl_t* nlist) {
	sl_elem_t *cursor;
	int count = 0;
	sl_foreach(nlist, cursor) {
		sl_elem_t *inlist = sl_find(orig, cursor->sl_str);
		if (inlist) {
			/* Already in the predecessor list */
			continue;
		}
		sl_elem_t *cp = sle_copy(inlist);
		sl_insert_head(orig, cp);
		count++;
	}
	return count;
}

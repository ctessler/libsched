#include "dag-task-set.h"

dtask_set_t *
dts_alloc() {
	dtask_set_t *task = calloc(1, sizeof(dtask_set_t));
	dts_init(task);

	return task;
}


int
dts_insert(dtask_set_t* head, dtask_elem_t *elem) {
	dts_insert_head(head, elem);
}

void
dts_clear(dtask_set_t *head) {
	dtask_elem_t *o_cursor, *tmp;

	for (o_cursor = dts_first(head); o_cursor;) {
		tmp = dts_next(o_cursor);
		/* Remove the item from the list */
		dts_remove(o_cursor);

		/* Free the task of the item */
		dtask_free(o_cursor->dts_task);

		/* Frees the item that was on the list */
		dtse_free(o_cursor);
		
		o_cursor = tmp;
	}
	dts_init(head);
}

/**
 * Finds the element of the deadline
 *
 * @param[in] head list head
 * @param[in] name the name of the task
 *
 * @return NULL if not found, the element otherwise.
 */
dtask_elem_t *
dts_find(dtask_set_t* head, char *name) {
	dtask_elem_t *cursor = NULL;

	for (cursor = dts_first(head); cursor; cursor = dts_next(cursor)) {
		if (strcmp(cursor->dts_task->dt_name, name) == 0) {
			break;
		}
	}
	
	return cursor;
}

/**
 * Allocate a new element
 *
 * Usage:
 *    dtask_t *task;
 *    ...
 *    dtask_update(task);
 *    dts_elem_t *item = dtse_alloc(task);
 */
dtask_elem_t*
dtse_alloc(dtask_t* task) {
	dtask_elem_t *elem = calloc(1, sizeof(dtask_elem_t));
	elem->dts_task = task;

	return elem;
}

void
dtse_free(dtask_elem_t* e) {
	free(e);
}


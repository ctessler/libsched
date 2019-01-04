#include "taskset.h"
static char BUFF[4096];

task_set_t*
ts_alloc() {
	task_set_t *ts = calloc(sizeof(task_set_t), 1);
	/* calloc ensures ts_head is NULL */
	return ts;
}

void
ts_free(task_set_t* ts) {
	task_link_t *head;
	while (head = ts->ts_head) {
		ts_rem(ts, head);
	}
	
}

char *
ts_string(task_set_t *ts) {
	return NULL;
}


task_link_t*
ts_add(task_set_t *ts, task_t *task) {
	task_link_t *cookie = calloc(sizeof(task_link_t), 1);
	cookie->tl_task = task;

	if (!ts->ts_head) {
		/* empty list */
		ts->ts_head = cookie;
		return cookie;
	}

	task_link_t *cursor = ts->ts_head;
	while (cursor->tl_next) {
		cursor = cursor->tl_next;
	}
	insque(cursor, cookie);

	return cookie;
}

task_link_t*
ts_find(task_set_t *ts, task_t *task) {
}

task_t*
ts_rem(task_set_t *ts, task_link_t *cookie) {
	/* NULL <- head <-> next <-> next -> NULL */
	task_t *task;
	remque(cookie);
	task = cookie->tl_task;
	free(cookie);

	return task;
}

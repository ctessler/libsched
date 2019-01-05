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
	task_link_t *cur, *next;
	for (cur = ts->ts_head; cur; cur = next) {
		next = cur->tl_next;
		ts_rem(ts, cur);
	}
	
}

char *
ts_string(task_set_t *ts) {
	task_link_t *cursor = ts->ts_head;
	char *s = BUFF;
	int n=0,count=1;
	for (cursor = ts->ts_head ; cursor ; cursor = cursor->tl_next, count++) {
		char *t = task_string(cursor->tl_task);
		if (cursor->tl_next) {
			n = sprintf(s, "%i: %s\n", count, t);
		} else {
			n = sprintf(s, "%i: %s", count, t);
		}
		free(t);
		s += n;
	}
	return strdup(BUFF);
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
	task_link_t *last = ts_last(ts);
	insque(cookie, last);
	
	return cookie;
}

task_link_t*
ts_find(task_set_t *ts, task_t *task) {

}

task_t*
ts_rem(task_set_t *ts, task_link_t *cookie) {
	task_t *task;
	remque(cookie);
	task = cookie->tl_task;
	free(cookie);

	return task;
}

task_link_t*
ts_last(task_set_t *ts) {
	task_link_t *cursor;
	for (cursor = ts->ts_head; cursor->tl_next; cursor = cursor->tl_next) {
		/* do nothing */
	}
	return cursor;
}

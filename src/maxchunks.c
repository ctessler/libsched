#include "maxchunks.h"

static void
assign_slack(task_set_t *ts, int64_t D, int64_t slack) {
	task_link_t *cookie;
	for (cookie = ts_first(ts) ; cookie ; cookie = ts_next(ts, cookie)) {
		task_t *task= ts_task(cookie);
		if (D != task->t_deadline) {
			return;
		}
		task->t_chunk = slack;
		if (task->t_chunk > task->wcet(task->t_threads)) {
			task->t_chunk = task->wcet(task->t_threads);
		}
	}
}

int
max_chunks(task_set_t *ts) {
	uint64_t star = ts_star(ts);

	ordl_t head;
	ordl_init(&head);
	ts_fill_deadlines(ts, &head, star);

	int feasible = 1;
	int64_t p_slack = INT64_MAX;
	or_elem_t *cursor;	
	ordl_foreach(&head, cursor) {
		int64_t D = cursor->oe_deadline;
		int64_t demand = ts_demand(ts, D);
		int64_t slack_d = D - demand;
		if (slack_d < p_slack) {
			p_slack = slack_d;
		}
		if (p_slack < 0) {
			feasible = 0;
			break;
		}
		task_t *task = cursor->oe_task;
		assign_slack(cursor->oe_tasks, D, p_slack);
	}
	or_elem_t *temp;
	for (cursor = ordl_first(&head); cursor; ) {
		temp = ordl_next(cursor);
		ordl_remove(cursor);
		oe_free(cursor);
		cursor = temp;
	}
	ordl_init(&head);

	if (feasible) {
		return 0;
	} else {
		return 1;
	}
}

int
max_chunks_nonp(task_set_t *ts) {
	task_link_t *cookie;
	task_t *t;
	uint32_t m, wcet, q;

	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		t = ts_task(cookie);
		q = t->t_chunk;
		m = t->t_threads;
		wcet = t->wcet(m);

		if (q < wcet) {
			/* 
			 * If any task has a non-preemptive chunk size less 
			 * than it's WCET it cannot be scheduled 
			 * non-preemptively
			 */
			return 1;
		}
	}
	return 0;
}

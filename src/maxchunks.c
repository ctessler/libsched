#include "maxchunks.h"

int
max_chunks(task_set_t *ts) {
	uint64_t star = ts_star(ts);
	int64_t demand = ts_demand(ts, star);

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
		if (D != task->t_deadline) {
			continue;
		}
		task->t_chunk = p_slack;
		if (task->t_chunk > task->wcet(task->t_threads)) {
			task->t_chunk =
				task->wcet(task->t_threads);
		}
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

#include "taskset-ot-deadlines.h"

tint_t
ts_fill_ot_deadlines_dbg(task_set_t *ts, ot_t *head, tint_t t, FILE *dbg) {
	task_link_t *cookie;
	task_t *task;
	int doclose = 0;
	tint_t count = 0;
	if (dbg == NULL) {
		dbg = fopen("/dev/null", "w");
		doclose = 1;
	}
	for (cookie = ts_first(ts); cookie; cookie = cookie->tl_next) {
		task = ts_task(cookie);
		fprintf(dbg, "%s: filling to %lu ... \n", task->t_name, t);
		fflush(dbg);
		tint_t thisc = ts_fill_ot_deadlines_task_dbg(task, head, t, dbg);
		fprintf(dbg, "%s: %lu deadlines added\n", task->t_name, thisc);
		count += thisc;
	}

	if (doclose) {
		fclose(dbg);
	}
	return count;

}
tint_t
ts_fill_ot_deadlines(task_set_t *ts, ot_t *head, tint_t t) {
	return ts_fill_ot_deadlines_dbg(ts, head, t, NULL);
}

tint_t
ts_fill_ot_deadlines_task_dbg(task_t *task, ot_t *head, tint_t t, FILE *dbg) {
	int doclose = 0;
	if (dbg == NULL) {
		dbg = fopen("/dev/null", "w");
		doclose = 1;
	}
	tint_t deadline, i = 0;
	ot_elem_t *D = NULL;
	tint_t est = ceil(t / (double) task->t_period);
	tint_t tenth = ceil(est / 10.0);
	fprintf(dbg, "Estimate: %lu, tenth: %lu\n", est, tenth);
	fprintf(dbg, "%s: [", task->t_name); fflush(dbg);
	do {
		deadline = task->t_deadline + (i * task->t_period);
		if (deadline <= t) {
			tint_t mod = i % tenth;
			D = ot_find(head, deadline);
			if (!D) {
				D = ote_alloc();
				D->ote_deadline = deadline;
				ot_ins(head, D);
				if (mod == 0) {
					fprintf(dbg, "+"); fflush(dbg);
				}
			} else {
				if (mod == 0) {				
					fprintf(dbg, "."); fflush(dbg);
				}
			}
			ts_add(D->ote_tasks, task);
		}
		i++;
	} while (deadline <= t);
	fprintf(dbg, "]\n");

	if (doclose) {
		fclose(dbg);
	}
	return i;
}	

tint_t
ts_fill_ot_deadlines_task(task_t *task, ot_t *head, tint_t t) {
	return ts_fill_ot_deadlines_task_dbg(task, head, t, NULL);
}


tint_t
ts_extend_ot_deadlines(task_set_t *ts, ot_t* head, tint_t prevb,
    tint_t newb) {
	if (prevb >= newb) {
		return 0;
	}
	task_link_t *cookie;
	task_t *task;
	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		task_t *task = ts_task(cookie);
		tint_t deadline, i=0;
		do {
			deadline = task->t_deadline + (i * task->t_period);
			i++;
			if (deadline <= prevb) {
				continue;
			}
			if (deadline <= newb) {
				ot_elem_t *D = ote_alloc();
				D->ote_deadline = deadline;
				ot_ins(head, D);
			}
		} while (deadline <= newb);
	}
}

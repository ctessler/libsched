#include "maxchunks.h"

static void
assign_slack(task_set_t *ts, int64_t D, int64_t slack) {
	task_link_t *cookie;
	char *str;
	for (cookie = ts_first(ts) ; cookie ; cookie = ts_next(ts, cookie)) {
		task_t *task= ts_task(cookie);
		if (D != task->t_deadline) {
			continue;
		}
		task->t_chunk = slack;
		if (task->t_chunk > task->wcet(task->t_threads)) {
			task->t_chunk = task->wcet(task->t_threads);
		}
	}
}

/**
 * An unfortunate consequence of the tsearch API. Global (file static
 * works) variables are required
 */
static struct {
	int64_t 	ad_pslack;
	int		ad_infeasible;
	task_set_t*	ad_tasks;
	FILE*		ad_dbg;
} ad_parms;

static void
act_deadline(const void *cookie, const VISIT which, const int depth) {
	ot_elem_t *elem = *(ot_elem_t**) cookie;

	switch (which) {
	case leaf:
	case postorder:
		break;
	default:
		return;
	}

	if (ad_parms.ad_infeasible) {
		return;
	}
	fprintf(ad_parms.ad_dbg, "%08li: demand=", elem->ote_deadline);
	fflush(ad_parms.ad_dbg);	
	int64_t D = elem->ote_deadline;
	int64_t demand = ts_demand(ad_parms.ad_tasks, D);
	int64_t slack_d = D - demand;
	fprintf(ad_parms.ad_dbg, "%08lu newslack=%08li", demand, slack_d);
	fflush(ad_parms.ad_dbg);
	if (slack_d < ad_parms.ad_pslack) {
		ad_parms.ad_pslack = slack_d;
	}
	if (ad_parms.ad_pslack < 0) {
		fprintf(ad_parms.ad_dbg, " infeasible, done\n");
		ad_parms.ad_infeasible = 1;
		return;
	}
	fprintf(ad_parms.ad_dbg, " assigning slack up to %08li\n", D);
	fflush(ad_parms.ad_dbg);	
	assign_slack(elem->ote_tasks, D, ad_parms.ad_pslack);
}

int
maxchunks_dbg(task_set_t *ts, FILE *dbg) {
	int closed = 0;
	if (dbg == NULL) {
		dbg = fopen("/dev/null", "w");
		closed = 1;
	}

	fprintf(dbg, "Calculating T* ... ");	fflush(dbg);
	uint64_t star = ts_star(ts);
	fprintf(dbg, "T* = %lu\n", star);

	ot_t *head = ot_alloc();
	fprintf(dbg, "Filling deadlines up to %lu\n", star);
	tint_t count = ts_fill_ot_deadlines_dbg(ts, head, star, dbg);
	fprintf(dbg, "Filled %lu deadlines\n", count);

	fprintf(dbg, "Beginning interval checks...\n");
	ad_parms.ad_pslack = INT64_MAX;
	ad_parms.ad_infeasible = 0;
	ad_parms.ad_tasks = ts;
	ad_parms.ad_dbg = dbg;
	twalk(head->ot_root, act_deadline);
	
	ot_empty(head);
	ot_free(head);
	if (closed) {
		fclose(dbg);
	}

	return ad_parms.ad_infeasible;
}

int
max_chunks_dbg(task_set_t *ts, FILE *handle) {
	int closeh = 0;
	if (handle == NULL) {
		handle = fopen("/dev/null", "w");
		closeh = 1;
	}
	fprintf(handle, "Calculating T* ... ");	fflush(handle);
	uint64_t star = ts_star(ts);
	fprintf(handle, "T* = %lu\n", star);

	ordl_t head;
	ordl_init(&head);
	fprintf(handle, "Filling deadlines up to %lu\n", star);
	int count = ts_fill_deadlines_dbg(ts, &head, star, handle);
	fprintf(handle, "Filled %i deadlines\n", count);
	

	fprintf(handle, "Beginning interval checks...\n");
	int feasible = 1;
	int64_t p_slack = INT64_MAX;
	or_elem_t *cursor;
	ordl_foreach(&head, cursor) {
		fprintf(handle, "%08li: demand=", cursor->oe_deadline); fflush(handle);
		int64_t D = cursor->oe_deadline;
		int64_t demand = ts_demand(ts, D);
		int64_t slack_d = D - demand;
		fprintf(handle, "%08lu newslack=%08li", demand, slack_d); fflush(handle);
		if (slack_d < p_slack) {
			p_slack = slack_d;
		}
		if (p_slack < 0) {
			fprintf(handle, " infeasible, done\n");
			feasible = 0;
			break;
		}
		fprintf(handle, " assigning slack up to %08li\n", D); fflush(handle);
		assign_slack(cursor->oe_tasks, D, p_slack);
	}
	ordl_clear(&head);

	if (closeh) {
		fclose(handle);
	}
	if (feasible) {
		fprintf(handle, "feasible\n");
		return 0;
	} else {
		return 1;
	}
}

int
max_chunks(task_set_t *ts) {
	return max_chunks_dbg(ts, NULL);
}

int
max_chunks_nonp(task_set_t *ts) {
	task_link_t *cookie;
	task_t *t;
	tint_t m, wcet, q;

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

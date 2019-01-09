#include "tpj.h"

/**
 * Modifies a task, such that the number of threads will complete
 * within slack amount of time.
 *
 * Additionally, the task set and ordered absolute deadline list which
 * contains the task is updated.
 *
 * @param[in|out] ts the task set, will be modified.
 * @param[in|out] head head of the ordered absolute deadline list
 * @param[in|out] the task being divided will have its threads and
 *     WCET values updated
 * @param[in] slack the available slack
 * 
 * @return non-zero upon success, zero otherwise
 */
static int
divide(task_set_t *ts, ordl_t *head, task_t *task, uint32_t slack) {
	char namebuf[TASK_NAMELEN], needle[TASK_NAMELEN], *p;
	task_link_t *ts_cursor = NULL;
	or_elem_t *oe_cursor = NULL;
	task_t *task_o,	*task_p;
	uint32_t threads;
	/*
	 * star_o - largest interval that needs to be checked with
	 *          current task set
	 * star_o - largest interval that needs to be checked after
	 *          dividing task into o_task and p_task
	 */
	uint64_t star_o = ts_star(ts);
	uint64_t star_p;

	for (int i=1; i <= task->t_threads; i++) {
		if (task->wcet(i) <= slack) {
			threads = i;
		}
		break;
	}
	/* threads now has the maximum size */
	if (threads == task->t_threads) {
		/* no change, shouldn't happen */
		return 0;
	}
	ts_cursor = ts_find(ts, task);
	if (!ts_cursor) {
		/* xxx-ct assert here. */
		return 0;
	}
	
	/* 
	 * Slightly wasteful, but easier to comprehend. The original
	 * task will be divided into tasks task_o and task_p.
	 * 
	 * task_o will replace the original task, taking on threads.
	 * task_p will be added to the sets, with the remaining
	 * threads.
	 */
	task_o = task_dup(task, threads);
	task_p = task_dup(task, task->t_threads - threads);
	snprintf(needle,TASK_NAMELEN,  "p[%u]", task->t_threads);
	p = strstr(task_p->t_name, needle);
	if (p) {
		sprintf(p, "p[%u]", task_p->t_threads);
	} else {
		snprintf(task_p->t_name, TASK_NAMELEN, "%s-p[%u]",
			 task->t_name, task_p->t_threads);
	}

	/* Remove the original task from the task set */
	ts_rem(ts, ts_cursor);

	/* Updating the ordered absolute deadline list, all the
	   previous entries need to be updated with the new task
	   (task_o) */
	ordl_foreach(head, oe_cursor) {
		if (oe_cursor->oe_task == task) {
			oe_cursor->oe_task = task_o;
		}
	}
	
	/* Free the original task */
	task_free(task);
	task = NULL;

	/* Add the "original" task back, but with fewer threads */
	ts_add(ts, task_o);

	/* Add the "new" task with the remaining threads into the set */
	ts_add(ts, task_p);
	/* 
	 * Insert the absolute deadlines for task_p into the ordered list of
	 * deadlines
	 */
	ts_fill_deadlines_task(task_p, head, star_o);

	/* Add new absolute deadlines if necessary */
	star_p = ts_star(ts);
	if (star_p > star_o) {
		ts_extend_deadlines(ts, head, star_o, star_p);
	}

	return 1;
}

int
tpj(task_set_t *ts, FILE *dbg) {
	uint64_t star = ts_star(ts);
	int64_t slack_b = INT64_MAX;	/* Previous intervals's slack value */
	int64_t slack_c = 0; 		/* Current interval's slack value */
	or_elem_t *cursor;
	int infeasible = 0;
	int doclose = 0;
	
	ordl_t head;
	ordl_init(&head);
	ts_fill_deadlines(ts, &head, star);

	if (!dbg) {
		dbg = fopen("/dev/null", "w");
		doclose = 1;
	}
	fprintf(dbg, "Absolute Deadlines:\n");
	ordl_foreach(&head, cursor) {
		fprintf(dbg, "%6u", cursor->oe_deadline);
	}
	fprintf(dbg, "\n");
	ordl_foreach(&head, cursor) {
		fprintf(dbg, "%6s", cursor->oe_task->t_name);
	}
	fprintf(dbg, "\n");

	ordl_foreach(&head, cursor) {
		uint32_t wcet, D_c;
		int64_t demand;
		task_t *task;
	repeat:
		task = cursor->oe_task;
		wcet = task->wcet(task->t_threads);
		D_c = cursor->oe_deadline;	/* Current interval */
		demand = ts_demand(ts, D_c); 
		slack_c = D_c - demand;		/* Slack at D_c */

		/* Minimum slack between b and c */
		if (slack_c < slack_b) {       
			slack_b = slack_c;
		}
		fprintf(dbg, "DBF(%3u):%-3lu Slack:%-3lu Task: %-8s WCET(%u):%u\n",
			D_c, demand, slack_b, task->t_name, task->t_threads, wcet);
		
		if (slack_b < 0) {
			/* Infeasible */
			infeasible = 1;
			break;
		}
		if (D_c != task->t_deadline)  {
			/* This is not the first job of task, nothing to do for q */
			continue;
		}
		if (slack_b < task->wcet(1)) {
			/* infeasible */
			infeasible = 1;
			break;
		}
		if (slack_b >= wcet) {
			/* There's enough slack to fit task without division */
			task->t_chunk = wcet;
			continue;
		}
		fprintf(dbg, "    WCET(%u):%u > Slack:%lu --> dividing %s\n",
			task->t_threads, wcet, slack_b, task->t_name);
		divide(ts, &head, task, slack_b);
		goto repeat;
	}

	/*
	 * Clean up the ORDL list, do *not* remove the tasks.
	 */
	for (cursor = ordl_first(&head); cursor;) {
		or_elem_t *tmp = ordl_next(cursor);
		ordl_remove(cursor);
		oe_free(cursor);
		cursor = tmp;
	}
	/* Play it safe, let the compiler throw out the call */	
	ordl_init(&head);

	fprintf(dbg, "\n");
	if (doclose) {
		fclose(dbg);
	}
	return infeasible;
}

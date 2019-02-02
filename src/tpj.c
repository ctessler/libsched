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
 * @param[in] star the T* to add deadlines of the ordered list to
 * 
 * @return non-zero upon success, zero otherwise
 */
static int
divide(task_set_t *ts, ordl_t *head, task_t *task, uint32_t slack, uint64_t star) {
	char namebuf[TASK_NAMELEN], needle[TASK_NAMELEN], *p;
	task_link_t *ts_cursor = NULL;
	or_elem_t *oe_cursor = NULL;
	task_t *task_o,	*task_p;
	uint32_t threads;

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
	 * ASSUMPTION
	 * TPJ will divide tasks *only* when encountering their
	 * relative deadline as an absolute deadline in the ordered
	 * list of deadlines. That is why it is safe to search with
	 * the relative deadline of the task
	 */
	oe_cursor = ordl_find(head, task->t_deadline);
	if (!oe_cursor) {
		/* xxx-ct assert here. */
		return 0;
	}

	/* Remove the original task from the task set specification */
	ts_rem(ts, ts_cursor);


	/* Counter intuitively, add posterior tasks first, remove
	   original task after. */
	uint32_t remaining = task->t_threads;
	while (remaining > 0) {
		task_t *task_n = NULL;
		if (remaining > threads) {
			task_n = task_dup(task, threads);
			task_n->t_chunk = task_n->wcet(threads);
			remaining -= threads;
		} else {
			task_n = task_dup(task, remaining);
			task_n->t_chunk = task_n->wcet(remaining);			
			remaining = 0;
		}
		/* Add the posterior tasks to task sets */
		ts_add(ts, task_n);
		/* Add the tasks to the deadlines, necessary for
		   demand */
	}
	/* Maintain the ordl set */
	ordl_rem_deadlines_task(head, task);
	task_free(task);
	
	return 1;
}

int
tpj(task_set_t *ts, FILE *dbg) {
	uint64_t star = ts_star(ts);
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

	uint32_t D_b = 0; 		/* Prev. interval */
	int64_t slack_b = INT64_MAX;	/* Prev. interval slack */
	ordl_foreach(&head, cursor) {
		uint32_t D_c = cursor->oe_deadline;	/* Cur. interval */
		int64_t slack_c, demand=0;
		int64_t slackp = slack_b;
		if (slackp == INT64_MAX) {
			slackp = -1;
		}

		task_set_t *deadline_ts = cursor->oe_tasks;
		task_link_t *cookie, *next;
		for (cookie = ts_first(deadline_ts); cookie; cookie = next) {
			next = ts_next(deadline_ts, cookie);
			task_t *task = ts_task(cookie);
			uint32_t wcet = task->wcet(task->t_threads);

			fprintf(dbg,
				"DBF(%3u):%-3lu Slack:%-3ld Task: %-8s WCET(%u):%u\n",
				D_b, demand, slackp, task->t_name, task->t_threads,
				wcet);

			if (D_c != task->t_deadline)  {
				/* This is not the first job of task,
				   nothing to do for q */ 
				continue;
			}
			if (slack_b < task->wcet(1)) {
				infeasible = 1;
				break;
			}
			if (slack_b >= wcet) {
				/* There's enough slack to fit task without division */
				task->t_chunk = wcet;
				fprintf(dbg,
					"    WCET(%u):%u < Slack:%ld --> assigning %s\n",
					task->t_threads, wcet, slackp, task->t_name);
				
				continue;
			}
			fprintf(dbg, "    WCET(%u):%u > Slack:%ld --> dividing %s\n",
				task->t_threads, wcet, slackp, task->t_name);
			divide(ts, &head, task, slack_b, star);
		}
		if (infeasible) {
			break;
		}
		
		slack_c = D_c - ts_demand(ts, D_c);
		if (slack_c < slack_b) {
			slack_b = slack_c;
		}
		D_b = D_c;
		if (slack_c < 0) {
			infeasible = 1;
			break;
		}
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

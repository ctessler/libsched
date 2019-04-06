#include"taskset-mod.h"

/**
 * Updates the number of threads assigned to the task
 *
 * Assumes t is in the task set ts, if necessary additional tasks with
 * one thread are added to ts -- when t does not have a WCET value for
 * 2 threads.
 *
 * @param[in|out] ts the task set t resides in
 * @param[in|out] t the task being modified
 * @param[in] m the number of threads of t
 */
int
tsm_update_threads(task_set_t* ts, task_t *t, tint_t m) {
	tint_t wcet = t->wcet(1);
	tint_t delta = abs(t->wcet(2) - t->wcet(1));

	task_threads(t, m);
	for (int i = 1; i <= m; i++) {
		t->wcet(i) = wcet + ((i - 1) * delta);
	}

	return 0;
}

int
tsm_dist_threads(gsl_rng *r, task_set_t* ts, int M, int minm, int maxm) {
	task_link_t *cookie;
	tint_t cap, count = 0;

	/* 
	 * Addition cap saves room for the minimum number of threads
	 * per task
	 */
	cap = M - (ts_count(ts) * minm);

	/* Add threads up to the first to try to violate the cap */
	cookie = ts_first(ts);
	while (count < cap) {
		task_t *t = ts_task(cookie);
		tint_t m = tsc_get_scaled(r, minm, maxm);
		if (count + m > cap) {
			/* time to min fill */
			break;
		}
		count += m;
		tsm_update_threads(ts, t, m);

		cookie = ts_next(ts, cookie);
		if (!cookie) {
			cookie = ts_first(ts);
		}
	}

	count = ts_threads(ts);
	/* Add the minimum number of threads per task up to M */
	while (count < M) {
		task_t *t = ts_task(cookie);
		tint_t m, cthreads = t->t_threads;

		if ((count + cthreads + minm) > M) {
			m = cthreads + 1;
			count += 1;
		} else {
			m = cthreads + minm;
			count += minm;
		}
		tsm_update_threads(ts, t, m);
		
		cookie = ts_next(ts, cookie);
		if (!cookie) {
			cookie = ts_first(ts);
		}
	}

	return 0;
}

static int
uu_update_task(task_set_t *ts, task_t *t, double u) {
	tint_t wcet = t->wcet(t->t_threads);

	t->t_period = (double) wcet / u;
}

int
tsm_uunifast_periods(gsl_rng *r, task_set_t* ts, double u, FILE *debug) {
	uunifast_cb(ts, u, r, debug, uu_update_task);

	return 0;
}

static tint_t
tsm_find_maxp(task_set_t* ts) {
	task_link_t *cookie;
	task_t *t;
	tint_t maxp = 0;
	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		t = ts_task(cookie);
		if (t->t_period > maxp) {
			maxp = t->t_period;
		}
	}
	return maxp;
}

int
tsm_set_deadlines(gsl_rng *r, task_set_t* ts, FILE *debug) {
	task_link_t *cookie;
	task_t *t;

	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		t = ts_task(cookie);
		tint_t min = t->wcet(t->t_threads);
		if (t->t_period / 2 > min) {
			min = t->t_period / 2;
		}
		tint_t deadline = tsc_get_scaled(r, min, t->t_period);
		t->t_deadline = deadline;
		if (!task_is_constrained(t)) {
			char *s = task_string(t);
			printf("Unconstrained task min[%lu]:\n%s\n", min, s);
			free(s);
		}
	}

	return 0;
}

int
tsm_force_concave(task_set_t *ts, FILE *debug) {
	task_link_t *cookie;
	task_t *t;

	task_set_t *scratch = ts_alloc();

	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		t = ts_task(cookie);
		if (t->t_threads <= 1) {
			continue;
		}
		tint_t delta = t->wcet(2) - t->wcet(1);
		if (delta < t->wcet(1)) {
			/* Already concave */
			continue;
		}
		tint_t wcet = t->wcet(1);
		tint_t total = t->t_threads;
		task_threads(t, 1);
		t->wcet(1) = wcet;
		for (int i = 2; i <= total; i++) {
			task_t *add = task_dup(t, 1);
			sprintf(add->t_name, "%s-%02d", t->t_name, i);
			ts_add(scratch, add);
		}
		sprintf(t->t_name, "%s-%02d", t->t_name, 1);
				
	}

	ts_move(scratch, ts);
	ts_destroy(scratch);
	return 0;
}

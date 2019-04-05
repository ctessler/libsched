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
tsm_update_threads(task_set_t* ts, task_t *t, uint32_t m) {
	uint32_t wcet = t->wcet(1);
	uint32_t delta = abs(t->wcet(2) - t->wcet(1));

	task_threads(t, m);
	for (int i = 1; i <= m; i++) {
		t->wcet(i) = wcet + ((i - 1) * delta);
	}

	return 0;
}

int
tsm_dist_threads(gsl_rng *r, task_set_t* ts, int M, int minm, int maxm) {
	task_link_t *cookie;
	uint32_t cap, count = 0;

	/* 
	 * Addition cap saves room for the minimum number of threads
	 * per task
	 */
	cap = M - (ts_count(ts) * minm);

	/* Add threads up to the first to try to violate the cap */
	cookie = ts_first(ts);
	while (count < cap) {
		task_t *t = ts_task(cookie);
		uint32_t m = tsc_get_scaled(r, minm, maxm);
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
		uint32_t m, cthreads = t->t_threads;

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

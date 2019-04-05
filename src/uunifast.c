#include "uunifast.h"

/*
 * Unfortunately the GSL does not provide a function that returns
 * values uniformly in the interval [0, 1], only [0, 1). This method
 * will return values [0, 1]
 */
static double
uu_get_scaled(gsl_rng *r) {
	uint64_t random;
	double scaled;

	random = gsl_rng_get(r);
	scaled =
		(double)(random - gsl_rng_min(r)) /
		(double)(gsl_rng_max(r) - gsl_rng_min(r));
	return scaled;
}

/**
 * Updates the task to meet the new usage u
 *
 * If the period is not set, then the WCET of the greatest number of
 * threads will be used to calculate the period to match the usage u.
 *
 * If the period is set, the number of threads are set to 1, and the
 * WCET to match the usage u is assigned.
 *
 * @param[in|out] t the task to be modified
 * @param[in] u the usage of t
 *
 * @return non-zero upon success, zero otherwise.
 */
static int
uu_update_task(task_set_t *ts, task_t *t, double u) {
	uint32_t wcet=0, p, m;

	p = t->t_period;
	m = t->t_threads;

	if (p == 0) {
		/* 0 period says, update the period */
		wcet = t->wcet(m);
		if (wcet <= 0) {
			/* Must have a WCET value */
			printf("uu_update_task called with incorrect task parms\n");
			return 0;
		}
		/* Period needs to be set to match max wcet */
		p = (double) wcet / u;
		t->t_period = ceil(p);
		return 1;
	}

	/*
	 * Update WCET for m threads. However, m might be zero, in
	 * that case increase to one.
	 */
	if (m <= 0) {
		m = 1;
	}
	task_threads(t, m);
	t->wcet(m) = ceil(p * u);

	return 1;
}

int
uunifast_cb(task_set_t *ts, double u, gsl_rng *r, FILE *debug, uu_updater callback) {
	double random, sum_u, sum_nu, c_u;
	int rv = 0, doclose = 0;
	uint32_t n_tasks;
	task_link_t *cookie;
	task_t *t;

	n_tasks = ts_count(ts);
	sum_u = u;
	cookie = ts_first(ts);
	t = ts_task(cookie);

	if (debug == NULL) {
		debug = fopen("/dev/null", "w");
		doclose = 1;
	}

	fprintf(debug, "Number of tasks: %u\n", n_tasks);
	for (int i=0; i < n_tasks; i++) {
		/* Perform the operations of UUniFast for task i */
		random = uu_get_scaled(r);
		sum_nu = sum_u * pow(random, (double)1/(n_tasks - i));
		c_u = sum_u - sum_nu;
		sum_u = sum_nu;

		/* Update the task in the set */
		if (!callback(ts, t, c_u)) {
			fprintf(debug, "Improper task!\n");
			rv = 1;
			goto bail;
		}
		/* Advance the task */
		cookie = ts_next(ts, cookie);
		if (cookie) {
			t = ts_task(cookie);
		}
	}
bail:
	if (doclose) {
		fclose(debug);
	}
	return rv;

}


int
uunifast(task_set_t *ts, double u, gsl_rng *r, FILE *debug) {
	return uunifast_cb(ts, u, r, debug, uu_update_task);
}

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
 * Updates the task to meet the new usage
 *
 * @param[in|out] t the task to be modified
 * @param[in] u the usage of t
 *
 * @return non-zero upon success, zero otherwise.
 */
static int
uu_update_task(task_t *t, double u) {
	uint32_t wcet=0, p, m;

	p = t->t_period;
	m = t->t_threads;
	if (m > 0) {
		wcet = t->wcet(m);
	}

	if (p == 0 && m == 0) {
		/* Can't do much with this */
		return 0;
	}
	
	if (p == 0) {
		/* Period needs to be set to match max wcet */
		p = (double) wcet / u;
		t->t_period = ceil(p);
		return 1;
	}

	task_threads(t, 1);
	t->wcet(1) = ceil(p * u);
	return 1;
}

int
uunifast(task_set_t *ts, double u, gsl_rng *r, FILE *debug) {
	double random, sum_u, sum_nu, c_u;
	uint32_t n_tasks;
	task_link_t *cookie;
	task_t *t;

	n_tasks = ts_count(ts);
	sum_u = u;
	cookie = ts_first(ts);
	t = ts_task(cookie);

	fprintf(debug, "Number of tasks: %u\n", n_tasks);
	for (int i=0; i < n_tasks; i++) {
		/* Perform the operations of UUniFast for task i */
		random = uu_get_scaled(r);
		sum_nu = sum_u * pow(random, (double)1/(n_tasks - i));
		c_u = sum_u - sum_nu;
		sum_u = sum_nu;

		/* Update the task in the set */
		if (!uu_update_task(t, c_u)) {
			printf("Improper task!\n");
			return 1;
		}
		/* Advance the task */
		cookie = ts_next(ts, cookie);
		if (cookie) {
			t = ts_task(cookie);
		}
	}
	return 0;
}




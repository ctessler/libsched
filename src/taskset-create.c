#include "taskset-create.h"

static uint32_t
tsc_get_scaled(gsl_rng *r, uint32_t min, uint32_t max) {
	unsigned long int random = gsl_rng_get(r);
	unsigned long int gsl_range = gsl_rng_max(r) - gsl_rng_min(r);
	uint32_t new_range = max - min + 1; /* Goes one past ex [5,10] --> [5,11] */
	double quotient =
		(double) ((random - gsl_rng_min(r)) * new_range) /
		(double) (gsl_range);
	uint32_t scaled = ceil(quotient) + min - 1; /* q can't be 0, so - 1 */

	return scaled;
}

static double
tsc_get_scaled_dbl(gsl_rng *r, double min, double max) {
	unsigned long int gsl_range;
	double random, new_range;

	gsl_range = gsl_rng_max(r) - gsl_rng_min(r);
	new_range = max - min;
	random = (gsl_rng_get(r) - gsl_rng_min(r)) * new_range;
	random = (random / gsl_range) +  min;

	return random;
}

static int
tsc_update_wcet_gf(task_t *t, double factor) {
	uint32_t m = t->t_threads;
	uint32_t wcet = t->wcet(m);
	double one = wcet / (1 + (m - 1) * factor);
	for (int i=1; i <= m; i++) {
		t->wcet(i) = ceil(one + (i - 1) * factor * one);
	}

	return 1;
}


int
tsc_bare_addn(task_set_t* ts, int n) {
	task_t *t;
	char name[TASK_NAMELEN];
	if (!ts) {
		/* Oopsie! */
		return 0;
	}
	for (int i=0; i < n; i++) {
		t = task_alloc(0, 0, 0);
		sprintf(name, "t:%i", i+1);
		task_name(t, name);
		if (!ts_add(ts, t)) {
			/* More oopsie! */
			return 0;
		}
	}
	return 1;
}

int
tsc_add_by_thread_count(task_set_t* ts, gsl_rng *r, uint32_t totalm,
    uint32_t minm, uint32_t maxm) {
	char name[TASK_NAMELEN];
	uint32_t count=0, m=0, i;
	task_t *t;

	for (i=1, count = 0; count < totalm; i++, count += m) {
		m = tsc_get_scaled(r, minm, maxm);
		if (count + m > totalm) {
			m = totalm - count;
		}
		t = task_alloc(0, 0, m);
		if (!t) {
			return 0;
		}
		if (!ts_add(ts, t)) {
			/* More oopsie! */
			return 0;
		}
		sprintf(name, "t:%i", i);
		task_name(t, name);
	}
	return count;
}

int
tsc_set_periods(task_set_t* ts, gsl_rng *r, uint32_t minp, uint32_t maxp) {
	task_link_t *cookie;
	for (cookie = ts_first(ts) ; cookie ; cookie = ts_next(ts, cookie)) {
		task_t *t = ts_task(cookie);
		t->t_period = tsc_get_scaled(r, minp, maxp);
	}
	return 1;
}

int
tsc_set_threads(task_set_t* ts, gsl_rng *r, uint32_t minm, uint32_t maxm) {
	task_link_t *cookie;
	for (cookie = ts_first(ts) ; cookie ; cookie = ts_next(ts, cookie)) {
		task_t *t = ts_task(cookie);
		task_threads(t, tsc_get_scaled(r, minm, maxm));
	}
	return 1;
}

int
tsc_set_deadlines_min_halfp(task_set_t *ts, gsl_rng *r, uint32_t mind, uint32_t maxd) {
	task_link_t *cookie;
	for (cookie = ts_first(ts) ; cookie ; cookie = ts_next(ts, cookie)) {
		task_t *t = ts_task(cookie);
		uint32_t c = t->wcet(t->t_threads);
		if (maxd < c) {
			/* Deadline cannot be less than the WCET */
			return 0;
		}
		uint32_t lmind = t->t_period / 2; /* p/2 */
		if (mind < c) {
			mind = c; /* min = max(c, p/2) */
		}
		if (lmind < mind ) {
			lmind = mind;
		}
		uint32_t lmaxd = maxd;
		if (lmaxd > t->t_period) {
			/* Must keep deadlines <= periods */
			lmaxd = t->t_period;
		}
		t->t_deadline = tsc_get_scaled(r, lmind, lmaxd);
	}
	return 1;
}


int
tsc_set_wcet_gf(task_set_t* ts, gsl_rng *r, float minf, float maxf) {
	task_link_t *cookie;
	for (cookie = ts_first(ts) ; cookie ; cookie = ts_next(ts, cookie)) {
		task_t *t = ts_task(cookie);
		if (t->t_threads <= 0) {
			/* Invalid task set */
			return 0;
		}
		double factor = tsc_get_scaled_dbl(r, minf, maxf);
		tsc_update_wcet_gf(t, factor);
	}
	return 1;
}

void ges_stfu() {
	
	int restore_fd = dup(fileno(stderr));
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-result"
	freopen("/dev/null", "w", stderr);
	#pragma GCC diagnostic pop

	/* STFU! */
	gsl_rng_env_setup();

	dup2(fileno(stderr), restore_fd);
}

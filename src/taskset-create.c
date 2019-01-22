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


int
tsc_bare_addn(task_set_t* ts, int n) {
	task_t *t;
	char name[TASK_NAMELEN];
	for (int i=0; i < n; i++) {
		t = task_alloc(0, 0, 0);
		sprintf(name, "t:%i", i+1);
		task_name(t, name);
		if (!ts) {
			/* Oopsie! */
			return 0;
		}
		if (!ts_add(ts, t)) {
			/* More oopsie! */
			return 0;
		}
	}
	return 1;
}

int
tsc_set_periods(task_set_t* ts, gsl_rng *r, uint32_t minp, uint32_t maxp) {
	task_link_t *cookie;
	for (cookie = ts_first(ts) ; cookie ; cookie = ts_next(ts, cookie)) {
		task_t *t = ts_task(cookie);
		t->t_period = tsc_get_scaled(r, minp, maxp);
	}
	
}

int
tsc_set_deadlines_min_halfp(task_set_t *ts, gsl_rng *r, uint32_t maxd) {
	task_link_t *cookie;
	for (cookie = ts_first(ts) ; cookie ; cookie = ts_next(ts, cookie)) {
		task_t *t = ts_task(cookie);
		//		t->t_period = tsc_get_scaled(r, minp, maxp);
	}
}

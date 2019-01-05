#include "taskset.h"
static char BUFF[4096];

task_set_t*
ts_alloc() {
	task_set_t *ts = calloc(sizeof(task_set_t), 1);
	/* calloc ensures ts_head is NULL */
	return ts;
}

void
ts_free(task_set_t* ts) {
	task_link_t *cur, *next;
	for (cur = ts->ts_head; cur; cur = next) {
		next = cur->tl_next;
		ts_rem(ts, cur);
	}
	free(ts);
	ts = NULL;
}

char *
ts_string(task_set_t *ts) {
	task_link_t *cursor = ts->ts_head;
	char *s = BUFF;
	memset(BUFF, 0, sizeof(BUFF));
	int n=0,count=1;
	for (cursor = ts->ts_head ; cursor ; cursor = cursor->tl_next, count++) {
		char *t = task_string(cursor->tl_task);
		if (cursor->tl_next) {
			n = sprintf(s, "%i: %s\n", count, t);
		} else {
			n = sprintf(s, "%i: %s", count, t);
		}
		free(t);
		s += n;
	}
	return strdup(BUFF);
}


task_link_t*
ts_add(task_set_t *ts, task_t *task) {
	task_link_t *cookie = calloc(sizeof(task_link_t), 1);
	cookie->tl_task = task;

	if (!ts->ts_head) {
		/* empty list */
		ts->ts_head = cookie;
		return cookie;
	}
	task_link_t *last = ts_last(ts);
	insque(cookie, last);
	
	return cookie;
}

task_link_t*
ts_find(task_set_t *ts, task_t *task) {
	task_link_t *cookie;
	task_t *t;
	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		if (t == task) {
			return cookie;
		}
	}
	return NULL;
}

task_t*
ts_rem(task_set_t *ts, task_link_t *cookie) {
	task_t *task = cookie->tl_task;
	remque(cookie); // frees the cookie

	return task;
}

task_link_t*
ts_last(task_set_t *ts) {
	task_link_t *cursor;
	for (cursor = ts->ts_head; cursor->tl_next; cursor = cursor->tl_next) {
		/* do nothing */
	}
	return cursor;
}

task_link_t*
ts_first(task_set_t *ts) {
	return ts->ts_head;
}

task_link_t*
ts_next(task_set_t *ts, task_link_t *cookie) {
	return cookie->tl_next;
}

/**
 * Could not find a gcd or lcm function in the standard C library.
 *
 * @return greatest common divisor of a and b
 */
static uint32_t
gcd(uint32_t a, uint32_t b) {
	uint32_t t;
	if (a < b) {
		t = a; a = b; b = t;
	}
	
	while (b != 0) {
		t = a % b;
		a = b;
		b = t;
	}
	return a;
}

/**
 * Could not find a gcd or lcm function in the standard C library.
 *
 * @return least common multiple of a and b
 */
static uint32_t
lcm(uint32_t a, uint32_t b) {
	// printf("gcd(%u, %u) = %u\n", a, b, gcd(a, b));
	double_t frac =  (double_t)( a / gcd(a, b) );
	return frac * b;
}

uint32_t
ts_hyperp(task_set_t *ts) {
	task_link_t *cookie = ts_first(ts);
	task_t *t;
	uint32_t P=0;

	if (!cookie) {
		return 0;
	}

	P = ts_task(cookie)->t_period;
	for (; cookie; cookie = cookie->tl_next) {
		t = ts_task(cookie);
		P = lcm(P, t->t_period);
	}
	return P;
}

uint32_t
ts_dmax(task_set_t *ts) {
	task_link_t *cookie = ts_first(ts);
	task_t *t;
	uint32_t dmax=0;

	if (!cookie) {
		return 0;
	}

	dmax = ts_task(cookie)->t_deadline;
	for (; cookie; cookie = cookie->tl_next) {
		t = ts_task(cookie);
		if (t->t_deadline > dmax) {
			dmax = t->t_deadline;
		}
	}
	return dmax;

}

float_t
ts_util(task_set_t *ts) {
	task_link_t *cookie;
	task_t *t;
	float_t U=0.0;

	for (cookie = ts_first(ts); cookie; cookie = cookie->tl_next) {
		t = ts_task(cookie);
		U += task_util(t);
	}

	return U;
}

uint64_t
ts_star(task_set_t *ts) {
	task_link_t *cookie;
	task_t *t;
	double_t upd = 0, updi;

	double_t max_r = 1 / (1 - ts_util(ts));
	for (cookie = ts_first(ts); cookie; cookie = cookie->tl_next) {
		t = ts_task(cookie);
		updi = task_util(t) * (t->t_period - t->t_deadline);
		upd += updi;
	}
	max_r *= upd;
	max_r = ceil(max_r);

	uint32_t D = ts_dmax(ts);
	double_t max = max_r;

	/* max(d_max, 1/(1-U) * sum(U * (p_i - d_i))) */
	if (D > max) {
		max = D;
	}
	
	/* min(P, max(...)) */
	uint64_t star = ts_hyperp(ts);
	if (star > max) {
		star = max;
	}
	return star;
}

char*
ts_permit(task_set_t* ts) {
	double_t u = ts_util(ts);
	u = 1 / (1 - u);
	if (isinf(u)) {
		sprintf(BUFF, "1 / (1 - U) of task set is infinite");
		return strdup(BUFF);
	}

	u = ts_util(ts);
	if (u >= 1) {
		sprintf(BUFF, "Utilization of the task set is >= 1, %f", u);
		return strdup(BUFF);
	}

	
	return NULL;
}

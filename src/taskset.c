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

void
ts_destroy(task_set_t* ts) {
	if (!ts) {
		return;
	}
	task_link_t *cur, *next;	
	for (cur = ts->ts_head; cur; cur = next) {
		next = cur->tl_next;
		task_t *task = ts_rem(ts, cur);
		task_free(task);
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
			n = sprintf(s, "%2i: %s\n", count, t);
		} else {
			n = sprintf(s, "%2i: %s", count, t);
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
	if (ts->ts_head && ts->ts_head == cookie) {
		ts->ts_head = cookie->tl_next;
		free(cookie);
		return task;
	}	
	remque(cookie); // frees the cookie .. unless it's the head.
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
	int32_t maxd = ts_max_pdiff(ts);

	double_t max_r = 1 / (1 - ts_util(ts));
	for (cookie = ts_first(ts); cookie; cookie = cookie->tl_next) {
		t = ts_task(cookie);
		updi = task_util(t) * maxd;
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

uint64_t
ts_star_debug(task_set_t *ts, FILE *f) {
	task_link_t *cookie;
	task_t *t;
	double_t upd = 0, updi;

	uint64_t P = ts_hyperp(ts);
	uint32_t D = ts_dmax(ts);
	float_t U = ts_util(ts);
	float_t x = 1 / (1 - U);
	int32_t diffmax = ts_max_pdiff(ts);	

	fprintf(f, "BEGIN T*\n");
	fprintf(f, "Max (p - d) = %i\n", diffmax);
	fprintf(f, "T*(tasks) = min(P, max(D))\n");
	fprintf(f, "T*(tasks) = min(%lu, max(D))\n", P);
	fprintf(f, "\tmax(D) = max(%u, Z)\n", D);
	fprintf(f, "\tZ = 1 / (1 - U) * sum\n");
	fprintf(f, "\tZ = 1 / (1 - %f) * sum\n", U);
	fprintf(f, "\tZ = %f * sum\n", x);
	fprintf(f, "\tsum = \n");


	float_t sum = 0;
	for (cookie = ts_first(ts); cookie; cookie = cookie->tl_next) {
		t = ts_task(cookie);
		float_t util = task_util(t);
		float_t s = util * diffmax;
		fprintf(f, "\t\t+ %s %f * %i = %f\n", t->t_name, util, diffmax, s);
		sum += s;
		
	}
	float_t Z = x * sum;
	fprintf(f, "\tZ = %f = %f * %f\n", Z, x, sum);
	fprintf(f, "\tZ = %f = ceil(%f)\n", ceil(Z), Z);
	Z = ceil(Z);
	uint32_t maxD = D;
	if (D < Z) {
		maxD = Z;
	}
	fprintf(f, "\tmax(D) = %u = max(%u, %f)\n", maxD, D, Z);

	uint64_t star = P;
	if (star > maxD) {
		star = maxD;
	}
	fprintf(f, "T*(tasks) = %lu = min(%lu, %u)\n", star, P, maxD);

	if (star != ts_star(ts)) {
		fprintf(f, "star functions return different values\n");
		exit(-1);
	}
	fprintf(f, "END T*\n");
	return star;
}

int32_t
ts_max_pdiff(task_set_t * ts) {
	task_link_t *cookie = NULL;
	task_t *t;
	int32_t maxd = 0, curd;

	for (cookie = ts_first(ts); cookie; cookie = cookie->tl_next) {
		t = ts_task(cookie);
		curd = t->t_period - t->t_deadline;
		if (curd > maxd) {
			maxd = curd;
		}
	}

	return maxd;
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

int64_t
ts_slack(task_set_t *ts, uint32_t t) {
	task_link_t *cookie;
	int64_t demand = ts_demand(ts, t);
	return 0;
}

int64_t
ts_demand(task_set_t *ts, uint32_t t) {
	task_link_t *cookie;
	task_t *task;
	int64_t demand = 0;
	for (cookie = ts_first(ts); cookie; cookie = cookie->tl_next) {
		task = ts_task(cookie);
		demand += task_dbf(task, t);
	}
	return demand;
}

int64_t
ts_demand_debug(task_set_t *ts, uint32_t t, FILE *f) {
	task_link_t *cookie;
	task_t *task;
	int64_t demand = 0;
	fprintf(f, "Task Set Demand at time %u\n", t);
	char *str = ts_string(ts); fprintf(f, "%s\n", str); free(str);
	for (cookie = ts_first(ts); cookie; cookie = cookie->tl_next) {
		task = ts_task(cookie);
		int64_t tdemand = task_dbf_debug(task, t, f);
		demand += tdemand;
		fprintf(f, "Total Demand: %li, %s adds %li demand\n",
			demand, task->t_name, tdemand);
	}
	fprintf(f, "ts_demand(%u) = %li\n", t, demand);

	if(demand != ts_demand(ts, t)) {
		fprintf(f, "Demand functions return different values\n");
		exit(-1);
	}
	return demand;
	
}

int
ts_fill_deadlines(task_set_t *ts, ordl_t *head, uint32_t t) {
	task_link_t *cookie;
	task_t *task;
	for (cookie = ts_first(ts); cookie; cookie = cookie->tl_next) {
		task = ts_task(cookie);
		ts_fill_deadlines_task(task, head, t);
	}
	return 1;
}

int
ts_fill_deadlines_task(task_t *task, ordl_t *head, uint32_t t) {
	uint32_t deadline, i = 0;
	or_elem_t *D = NULL;
	do {
		deadline = task->t_deadline + (i * task->t_period);
		if (deadline <= t) {
			D = oe_alloc();
			D->oe_deadline = deadline;
			D->oe_task = task;
			ordl_insert(head, D);
		}
		i++;
	} while (deadline <= t);

	return i;
}

int
ts_extend_deadlines(task_set_t *ts, ordl_t* head, uint32_t prevb,
    uint32_t newb) {
	if (prevb >= newb) {
		return 0;
	}
	task_link_t *cookie;
	task_t *task;
	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		task_t *task = ts_task(cookie);
		uint32_t deadline, i=0;
		do {
			deadline = task->t_deadline + (i * task->t_period);
			i++;
			if (deadline <= prevb) {
				continue;
			}
			if (deadline <= newb) {
				or_elem_t *D = oe_alloc();
				D->oe_deadline = deadline;
				D->oe_task = task;
				ordl_insert(head, D);
			}
		} while (deadline <= newb);
	}
}

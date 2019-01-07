#include <stdio.h>
#include "taskset.h"

void clean_task_set(task_set_t *ts);


int test_alloc();
int test_add_fill();
void test_add_one();
void test_fill(task_set_t *ts);
void test_star();
void test_demand();
void test_delta();
void test_sanjoy();
void test_sanjoy_star();

int
main(int argc, char** argv) {
	test_alloc();
	test_add_one();
	test_add_fill();
	test_star();
	test_demand();
	test_sanjoy();
	test_sanjoy_star();	
	return 0;
}

void
clean_task_set(task_set_t *ts) {
	task_link_t *cookie;
	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		task_free(ts_task(cookie));
		ts_task(cookie) = NULL;
		ts_rem(ts, cookie);
	}
	ts_free(ts);
}

int
test_alloc() {
	task_set_t *ts = ts_alloc();
	ts_free(ts);
}

void
test_add_one() {
	task_set_t *ts = ts_alloc();
	task_t *task = task_alloc(10, 20, 3);
	task->wcet(1) = 25;
	task->wcet(2) = 35;
	task->wcet(3) = 35;

	ts_add(ts, task);

	char *str = ts_string(ts);
	printf("%s\n", str);
	free(str);
	
	ts_free(ts);
	task_free(task);
}

int
test_add_fill() {
	task_set_t *ts = ts_alloc();

	test_fill(ts);

	char *str = ts_string(ts);
	printf("%s\n", str);
	free(str);

	printf("Hypeperiod %u\n", ts_hyperp(ts));
	printf("Maximum Deadline %u\n", ts_dmax(ts));
	printf("Utilization %.05f\n", ts_util(ts));
	printf("T^*(set) %lu\n", ts_star(ts));

	task_link_t *cookie;
	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		task_free(ts_task(cookie));
		ts_task(cookie) = NULL;
		ts_rem(ts, cookie);
	}
	ts_free(ts);
}

void
test_fill(task_set_t *ts) {
	task_t *task = task_alloc(140, 130, 3);
	task->wcet(1) = 25;
	task->wcet(2) = 35;
	task->wcet(3) = 35;
	ts_add(ts, task);

	task = task_alloc(170, 160, 3);
	task->wcet(1) = 35;
	task->wcet(2) = 55;
	task->wcet(3) = 65;
	ts_add(ts, task);

	task = task_alloc(95, 90, 2);
	task->wcet(1) = 10;
	task->wcet(2) = 12;
	task->wcet(3) = 16;
	ts_add(ts, task);

	task = task_alloc(100, 80, 3);
	task->wcet(1) = 10;
	task->wcet(2) = 12;
	task->wcet(3) = 16;
	ts_add(ts, task);

	char *str = ts_string(ts);
	printf("%s\n", str);
	free(str);

	str = ts_permit(ts);	
	if (str) {
		printf("%s\n", str);
		free(str);
		exit(-1);
	}

}


void
test_star() {
	task_set_t *ts = ts_alloc();
	task_t *t;

	printf("Verifying T^*(set)\n");
	t = task_alloc(200, 180, 1);
	t->wcet(1) = 50;
	ts_add(ts, t);

	t = task_alloc(200, 180, 1);
	t->wcet(1) = 50;
	ts_add(ts, t);

	t = task_alloc(200, 180, 1);
	t->wcet(1) = 50;
	ts_add(ts, t);

	t = task_alloc(202, 180, 1);
	t->wcet(1) = 50;
	ts_add(ts, t);

	char *str = ts_permit(ts);
	if (str) {
		printf("%s\n", str);
		free(str);
		str = ts_string(ts);
		printf("%s\n", str);
		free(str);
		exit(-1);
	}

	str = ts_string(ts);
	printf("%s\n", str);
	free(str);

	double_t star = ts_star(ts);
	printf("T^*(set): %f\n", star);
	if (star == 8260) {
		printf("\tCorrect!\n");
	} else {
		printf("\tIncorrect!\n");
		exit(-1);
	}

	t->t_period = 200;
	t->wcet(1) = 30;
	star = ts_star(ts);
	printf("T^*(set): %f\n", star);
	if  (star == 180) {
		printf("\tCorrect!\n");
	} else {
		printf("\tIncorrect!\n");
		exit(-1);
	}
	clean_task_set(ts);

	ts = ts_alloc();
	t = task_alloc(200, 100, 1);
	t->wcet(1) = 100;
	ts_add(ts, t);

	t = task_alloc(30, 30, 1);
	t->wcet(1) = 14;
	ts_add(ts, t);

	str = ts_permit(ts);
	if (str) {
		printf("%s\n", str);
		free(str);
		str = ts_string(ts);
		printf("%s\n", str);
		free(str);
		exit(-1);
	}
	
	star = ts_star(ts);
	printf("T^*(set): %f\n", star);
	
	clean_task_set(ts);	
	ts = NULL;
}


void
demand_fill(task_set_t *ts) {
	// t_1 8, 8, 2
	task_t *task = task_alloc(8, 8, 1); task->wcet(1) = 2;
	task_name(task, "t[1]");
	ts_add(ts, task);

	// t_2 20, 10, 4
	task = task_alloc(20, 10, 1); task->wcet(1) = 4;
	task_name(task, "t[2]");	
	ts_add(ts, task);

	// t_3 25, 15, 2
	task = task_alloc(25, 15, 1); task->wcet(1) = 2;
	task_name(task, "t[3]");
	ts_add(ts, task);

	// t_4 35, 30, 4
	task = task_alloc(35, 30, 1); task->wcet(1) = 4;
	task_name(task, "t[4]");
	ts_add(ts, task);

	// t_5 50, 50, 4
	task = task_alloc(50, 50, 1); task->wcet(1) = 3;
	task_name(task, "t[5]");
	ts_add(ts, task);

	// t_6 90, 50, 4
	task = task_alloc(90, 50, 1); task->wcet(1) = 4;
	task_name(task, "t[6]");
	ts_add(ts, task);

	// t_7 110, 60, 8
	task = task_alloc(110, 60, 1); task->wcet(1) = 8;
	task_name(task, "t[7]");
	ts_add(ts, task);

	// t_8 105, 60, 5
	task = task_alloc(105, 60, 1); task->wcet(1) = 5;
	task_name(task, "t[8]");
	ts_add(ts, task);

	// t_9 100, 60, 3
	task = task_alloc(100, 60, 1); task->wcet(1) = 3;
	task_name(task, "t[9]");
	ts_add(ts, task);

	// t_10 110, 100, 4
	task = task_alloc(110, 100, 1); task->wcet(1) = 4;
	task_name(task, "t[10]");
	ts_add(ts, task);

	char *str = ts_string(ts);
	printf("%s\n", str);
	free(str);

	str = ts_permit(ts);	
	if (str) {
		printf("%s\n", str);
		free(str);
		exit(-1);
	}
}

void test_demand() {
	task_set_t *ts = ts_alloc();
	printf("Testing Task Set Demand\n");
	demand_fill(ts);
	uint64_t star = ts_star(ts);
	printf("T*(ts) = %ld\n", star);

	int64_t demand = ts_demand(ts, star);
	printf("Demand: %li\n", demand);

	ordl_t head;
	ordl_init(&head);

	ts_fill_deadlines(ts, &head, star);
	printf("Absolute deadlines: ");
	or_elem_t *cursor;
	ordl_foreach(&head, cursor) {
		printf("%u", cursor->oe_deadline);
		if (ordl_next(cursor)) {
			printf(", ");
		}
	}
	printf("\n");

	int feasible = 1;
	int64_t p_slack = INT64_MAX;
	ordl_foreach(&head, cursor) {
		int64_t D = cursor->oe_deadline;
		printf("Deadline %2u(%5s) ", cursor->oe_deadline,
		    cursor->oe_task->t_name);
		int64_t demand = ts_demand(ts, D);
		printf("Demand %2li ", demand);
		int64_t slack_d = D - demand;
		if (slack_d < p_slack) {
			p_slack = slack_d;
		}
		printf("Slack %li ", p_slack);
		if (p_slack < 0) {
			feasible = 0;
			break;
		}
		task_t *task = cursor->oe_task;
		if (D == task->t_deadline) {
			task->t_chunk = p_slack;
			if (task->t_chunk > task->wcet(task->t_threads)) {
				task->t_chunk = task->wcet(task->t_threads);
			}
			printf("Task %s chunk %u", task->t_name, task->t_chunk);
		}
		printf("\n");
	}
	if (feasible) {
		printf("feasible\n");
	} else {
		printf("infeasible\n");
	}
	
	
	clean_task_set(ts);
	ts = NULL;
}

void test_sanjoy() {
	task_set_t *ts = ts_alloc();
	printf("Sanjoy Example Test at D = 60\n");
	demand_fill(ts);

	int64_t demand = ts_demand_debug(ts, 60, stdout);

	task_link_t *cookie;
	int64_t d2 = 0;
	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		task_t* task = cookie->tl_task;
		printf("DBF(%5s, 60) = %u\n", task->t_name,
		       task_dbf(task, 60));
		d2 += task_dbf(task, 60);
	}
	printf("--------------------\n");
	printf("  DBF(all, 60) = %li\n", d2);
	
	
	clean_task_set(ts);
	ts = NULL;
}

void test_sanjoy_star() {
	task_set_t *ts = ts_alloc();
	printf("Sanjoy Example T* = 725\n");
	demand_fill(ts);

	uint64_t star = ts_star_debug(ts, stdout);
	
	
	clean_task_set(ts);
	ts = NULL;
}

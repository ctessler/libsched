#include <stdio.h>
#include "taskset.h"

void clean_task_set(task_set_t *ts);


int test_alloc();
int test_add_fill();
void test_add_one();
void test_fill(task_set_t *ts);
void test_star();

int
main(int argc, char** argv) {
	test_alloc();
	test_add_one();
	test_add_fill();
	test_star();
	return 0;
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
clean_task_set(task_set_t *ts) {
	task_link_t *cookie;
	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		task_free(ts_task(cookie));
		ts_task(cookie) = NULL;
		ts_rem(ts, cookie);
	}
	ts_free(ts);
}

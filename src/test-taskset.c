#include <stdio.h>
#include "taskset.h"

int test_alloc();
int test_add_fill();
void test_add_one();
void test_fill(task_set_t *ts);

int
main(int argc, char** argv) {
	test_alloc();
	test_add_one();
	test_add_fill();
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
	task_t *task = task_alloc(10, 50, 3);
	task->wcet(1) = 25;
	task->wcet(2) = 35;
	task->wcet(3) = 35;
	ts_add(ts, task);

	task = task_alloc(15, 100, 3);
	task->wcet(1) = 35;
	task->wcet(2) = 55;
	task->wcet(3) = 65;
	ts_add(ts, task);

	task = task_alloc(5, 40, 2);
	task->wcet(1) = 10;
	task->wcet(2) = 12;
	task->wcet(3) = 16;
	ts_add(ts, task);

	task = task_alloc(22, 70, 2);
	task->wcet(1) = 10;
	task->wcet(2) = 12;
	task->wcet(3) = 16;
	ts_add(ts, task);

}

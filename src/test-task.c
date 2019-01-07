#include <stdio.h>
#include "task.h"

int test_alloc();
int test_threads();
int test_dbf();
int test_utilization();

int
main(int argc, char** argv) {
	test_alloc();
	test_threads();
	test_utilization();
	test_dbf();	
	return 0;
}

int test_alloc() {
	task_t *task = NULL;
	char *str = NULL;

	printf("Testing the task.\n");
	task = task_alloc(10, 20, 3);
	task->wcet(1) = 10;
	task->wcet(2) = 15;
	task->wcet(3) = 20;

	str = task_string(task);
	printf("%s\n", str);
	free(str);
	
	task_free(task);

	return 0;
}

int test_threads() {
	task_t *task = NULL;
	char *str = NULL;

	printf("Testing the task.\n");
	task = task_alloc(10, 20, 3);
	task_threads(task, 5);

	task->wcet(1) = 25;
	task->wcet(2) = 35;
	task->wcet(3) = 45;	
	task->wcet(4) = 55;
	task->wcet(5) = 65;	
	
	str = task_string(task);
	printf("%s\n", str);
	free(str);
	
	task_free(task);

}

int
test_utilization() {
	task_t *task = NULL;
	char *str;

	task = task_alloc(30, 25, 3);
	task->wcet(1) = 10;
	task->wcet(2) = 15;
	task->wcet(3) = 20;

	str = task_string(task);
	printf("%s u:%.02f\n", str, task_util(task));
	free(str);

	task_free(task);
}

int
test_dbf() {
	task_t *task = task_alloc(30, 20, 3);
	task->wcet(1) = 10;
	task->wcet(2) = 12;
	task->wcet(3) = 14;

	char* str = task_string(task);
	printf("%s u:%.02f\n", str, task_util(task));
	free(str);
	uint32_t t = 20, e=14;
	printf("dbf(%u) = %u (should be %u)\n", t, task_dbf(task, t), e);
	if (task_dbf(task, t) != e) {
		exit(-1);
	}

	t = 19; e = 0;
	printf("dbf(%u) = %u (should be %u)\n", t, task_dbf(task, t), e);
	if (task_dbf(task, t) != e) {
		exit(-1);
	}

	t = 0; e = 0;
	printf("dbf(%u) = %u (should be %u)\n", t, task_dbf(task, t), e);
	if (task_dbf(task, t) != e) {
		exit(-1);
	}

	t = 49; e = 14;
	printf("dbf(%u) = %u (should be %u)\n", t, task_dbf(task, t), e);
	if (task_dbf(task, t) != e) {
		exit(-1);
	}
	
	t = 50; e = 28;
	printf("dbf(%u) = %u (should be %u)\n", t, task_dbf(task, t), e);
	if (task_dbf(task, t) != e) {
		exit(-1);
	}

	t = 79; e = 28;
	printf("dbf(%u) = %u (should be %u)\n", t, task_dbf(task, t), e);
	if (task_dbf(task, t) != e) {
		exit(-1);
	}

	t = 80; e = 42;
	printf("dbf(%u) = %u (should be %u)\n", t, task_dbf(task, t), e);
	if (task_dbf(task, t) != e) {
		exit(-1);
	}
	task_free(task);
}

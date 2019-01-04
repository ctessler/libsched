#include <stdio.h>
#include "task.h"

int test_alloc();
int test_threads();

int
main(int argc, char** argv) {
	test_alloc();
	test_threads();
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

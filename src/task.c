#include <task.h>
static char BUFF[512];

task_t*
task_alloc(uint32_t period, uint32_t deadline, uint32_t threads) {
	task_t* task = calloc(sizeof(task_t), 1);
	task_threads(task, threads);
	task->t_period = period;
	task->t_deadline = deadline;
	return task;
}

void
task_free(task_t* task) {
	if (!task) {
		return;
	}
	free(task->t_wcet);
	task->t_wcet = NULL;
	free(task);
}

int
task_threads(task_t *task, uint32_t threads) {
	if (task->t_wcet) {
		free(task->t_wcet);
	}
	task->t_threads = threads;
	task->t_wcet = calloc(sizeof(uint32_t), threads);
}

char *
task_string(task_t *task) {
	char *s = BUFF;
	int n = sprintf(BUFF, "(p:%u, d:%u, m:%u) wcet {",
	    task->t_period, task->t_deadline, task->t_threads);
	s = BUFF;
	for (int i=1; i <= task->t_threads; i++) {
		s += n;
		n = sprintf(s, "%u", task->wcet(i));
		if (i < task->t_threads) {
			s += n;
			n = sprintf(s, ", ");
		}
	}
	s += n;
	sprintf(s, "}");
	return strdup(BUFF);
}

float_t
task_util(task_t *task) {
	uint32_t m = task->t_threads;
	uint32_t c = task->wcet(m);

	return (float_t)((float_t) c / task->t_period);
}

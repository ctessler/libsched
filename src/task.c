#include <task.h>
static char BUFF[1024];

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

void
task_name(task_t* task, const char* name) {
	strncpy(task->t_name, name, TASK_NAMELEN);
}

char *
task_string(task_t *task) {
	char *s = BUFF;
	int n = sprintf(BUFF, "(p:%4u, d:%4u, m:%2u) wcet {",
	    task->t_period, task->t_deadline, task->t_threads);
	s = BUFF;
	for (int i=1; i <= task->t_threads; i++) {
		s += n;
		n = sprintf(s, "%4u", task->wcet(i));
		if (i < task->t_threads) {
			s += n;
			n = sprintf(s, ", ");
		}
	}
	s += n;
	n = sprintf(s, "} ");
	s += n;
	sprintf(s, "u:%.3f q:%u, %s", task_util(task), task->t_chunk, task->t_name);
	return strdup(BUFF);
}

float_t
task_util(task_t *task) {
	uint32_t m = task->t_threads;
	uint32_t c = task->wcet(m);

	return (float_t)((float_t) c / task->t_period);
}

uint32_t
task_dbf(task_t *task, uint32_t t) {
	if (t < task->t_deadline) {
		return 0;
	}
	uint32_t j = t - task->t_deadline;
	j = j / task->t_period;
	j += 1;
	j *= task->wcet(task->t_threads);

	return j;
}

uint32_t
task_dbf_debug(task_t *task, uint32_t t, FILE *f) {
	if (t < task->t_deadline) {
		fprintf(f, "DBF(%s, t = %u) = 0 ", task->t_name, t);
		fprintf(f, ": t < deadline (%u < %u)\n", t, task->t_deadline);
		return 0;
	}
	uint32_t numerator = t - task->t_deadline;
	uint32_t denominator = task->t_period;
	uint32_t wcet = task->wcet(task->t_threads);
	uint32_t frac = numerator / denominator;
	uint32_t demand = (frac + 1) * wcet;
	fprintf(f, "DBF(%s, t = %u) = %u\n", task->t_name, t, demand);
	fprintf(f, "        | %5u - %-5u     |\n", t, task->t_deadline);
	fprintf(f, "%5u * | ------------- + 1 | = %u * (%u + 1) = %5u\n", wcet,
		wcet, frac, demand);
	fprintf(f, "        |  %8u         |\n", task->t_period);
	fprintf(f, "        +-                 -+\n");
	

	return demand;
}

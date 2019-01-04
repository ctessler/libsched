#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	uint32_t t_period;
	uint32_t t_deadline;
	uint32_t t_threads;
	uint32_t *t_wcet;
} task_t;

/**
 * WCET of n threads
 *
 * @param[in] the number of threads
 *
 * Usage:
 *     task_t t;
 *     t.wcet(1) = 25;
 *     t.wcet(2) = 35;
 *
 *     printf("WCET of 2 threads, %u\n", t.wcet(2));
 * 
 */
#define wcet(n) t_wcet[n-1]

/**
 * Allocates a new task
 *
 * @note a task must be task_free()'d 
 *
 * @param[in] period the minimum inter-arrival time of a job
 * @param[in] deadline the relative deadline
 * @param[in] threads the number of threads released with each job
 *
 * @return the new task
 */
task_t* task_alloc(uint32_t period, uint32_t deadline, uint32_t threads);
void task_free(task_t *task);


/**
 * Updates the number of threads a task releases with each job
 *
 * Usage:
 *     task_t *t = task_alloc(10, 20, 3);
 *     task_threads(t, 5);
 *     t->wcet(1) = 3;
 *     t->wcet(2) = 5;
 * 
 * @note changing the number of threads destroys the WCET table
 *
 * @param[in] threads the new number of threads released with each job
 */
int task_threads(task_t *task, uint32_t threads);

/**
 * Returns a dynamically allocated string representing the task
 *
 * @note the string must be free()'d
 */
char *task_string(task_t *task);

#endif /* TASK_H */

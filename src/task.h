#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TASK_NAMELEN 64

/**
 * tint_t a type for integer task parameters
 * 
 * This is necessary because overflow will happen with 32 bit unsigned
 * integers and even 64 bit integers for large task sets. Redefine the
 * type for more storage.
 */
typedef uint64_t tint_t;

typedef struct {
	char t_name[TASK_NAMELEN];
	tint_t t_period;
	tint_t t_deadline;
	tint_t t_threads;
	tint_t t_chunk;	/**< Maximum non-preepmtive chunk (q) */
	tint_t *t_wcet;
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
 * A task may be created with a peroid, deadline, and number of
 * threads equal to zero. 
 *
 * @param[in] period the minimum inter-arrival time of a job
 * @param[in] deadline the relative deadline
 * @param[in] threads the number of threads released with each job
 *
 * @return the new task
 */
task_t* task_alloc(tint_t period, tint_t deadline, tint_t threads);
void task_free(task_t *task);

/**
 * Duplicates a task
 *
 * Creates a new task from the original, assigning the WCET values of
 * the first t threads in the new task from the original.
 *
 * @param[in] orig the task being duplicated
 * @param[in] t the number of threads in the new duplicate task
 *
 * @return a new task upon success which must be task_free()'d, NULL otherwise. 
 */
task_t* task_dup(task_t *orig, tint_t t);

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
 *
 * @return the number of threads in the resized task
 */
int task_threads(task_t *task, tint_t threads);

/**
 * Updates the name of the task
 *
 * @param[in] task the task
 * @param[in] name the name of the task
 */
void task_name(task_t *task, const char* name);

/**
 * Returns a dynamically allocated string representing the task
 *
 * @note the string must be free()'d
 */
char *task_string(task_t *task);

/**
 * Returns a dynamically allocated string to head the task
 *
 * @note the string must be free()'d
 */
char *task_header(task_t *task);

/**
 * Calculates the utilization for the task
 *
 * @param[in] task the task
 *
 * @return the utilization of the task
 */
float_t task_util(task_t *task);

/**
 * Calculates the maximum demand for the task over t time units
 *
 * @param[in] task the task
 * @param[in] t the units of time
 *
 * @return the maximum demand of task over t time units
 */
tint_t task_dbf(task_t *task, tint_t t);
tint_t task_dbf_debug(task_t *task, tint_t t, FILE *f);

/**
 * Merges a task from m to 1 thread with maximum WCET
 * 
 * @param[in|out] task the task
 */
int task_merge(task_t* task);

/**
 * Determines if th task has a constrained deadlines (d <= p)
 *
 * @param[in] task the task 
 * 
 * @return non-zero if the task is constrained, zero otherwise
 */
int task_is_constrained(task_t* task);
   
#endif /* TASK_H */

#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "taskset-config.h"
#include "taskset-create.h"
#include "uunifast.h"

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_lname;
	char* c_oname;
	char* c_fname;	
	tint_t c_minm;
	tint_t c_maxm;
} clc;

enum {
      ARG_MINM = CHAR_MAX + 1,
      ARG_MAXM,
};

static const char* short_options = "hl:o:s:v";
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"log", required_argument, 0, 's'},
    {"maxm", required_argument, 0, ARG_MAXM},
    {"output", required_argument, 0, 'o'},
    {"task-set", required_argument, 0, 's'},
    {"verbose", no_argument, &clc.c_verbose, 1},
    {0, 0, 0, 0}
};

void
usage() {
	printf("ts-divide: Task Set Divider for maximum number of threads\n");
	printf("Usage: ts-divide [OPTIONS] -s <FILE> --maxm <INT>\n");
	printf("OPTIONS:\n");
	printf("\t--help/-h\t\tThis message\n");
	printf("\t--log/-l <FILE>\t\tAuditible log file\n");
	printf("\t--maxm <INT>\t\tMaximum number of threads per task\n");
	printf("\t--output/-o <FILE>\tOutput file of new task set\n");
	printf("\t--task-set/-s <FILE>\tTask set configuration file\n"); 	
	printf("\t--verbose/-v\t\tEnables verbose output\n");
	printf("\nOPERATION:\n");
	printf("\tTasks will be divided into subsequent tasks of at most --maxm");
	printf(" threads per job\n");
	printf("\nREQUIREMENTS:\n");
	printf("\tEach threads per job WCET value must be assigned\n");
	printf("\nEXAMPLES:\n");
	printf("\tDivide tasks into tasks with at most 3 threads\n");
	printf("\t> ts-divide --maxm 3 -s taskset.ts\n\n");
}

int
check_ts(task_set_t *ts) {
	if (!ts) {
		printf("No task set\n");
		return 0;
	}
	task_link_t *cookie;
	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		task_t *t = ts_task(cookie);
		tint_t m = t->t_threads;
		if (m <= 0) {
			continue;
		}
		for (int j=1; j <=m; j++) {
			tint_t wcet = t->wcet(j);
			if (wcet <= 0) {
				fprintf(stderr,
					"Error: task %s has %lu threads and a WCET"
					" for %u threads of %lu", t->t_name,
					m, j, wcet);
				return 0;
			}
		}
	}
	return 1;
}

int
main(int argc, char** argv) {
	task_set_t *ts = NULL;
	FILE *ofile = stdout;
	config_t cfg;
	int rv = 0;

	/* Initilialize the config object */
	config_init(&cfg);
	/*
	 * Initializer for the GNU Scientific Library for random numbers
	 * Suggested values for environment variables
	 *   GSL_RNG_TYPE=ranlxs2
	 *   GSL_RNG_SEED=`date +%s`
	 */
	ges_stfu();
	while(1) {
		int opt_idx = 0;
		int c = getopt_long(argc, argv, short_options,
		    long_options, &opt_idx);
		if (c == -1) {
			break;
		}

		switch(c) {
		case 0:
			break;
		case 'h':
			usage();
			goto bail;
		case 'l':
			/* Needs to be implemented */
			printf("Log file not implemented\n");
			usage();
			goto bail;
		case 'o':
			clc.c_oname = strdup(optarg);
			break;
		case 's':
			clc.c_fname = strdup(optarg);
			break;
		case 'v':
			clc.c_verbose = 1;
			break;
		case ARG_MINM: /* minp */
			clc.c_minm = atoi(optarg);
			break;
		case ARG_MAXM: /* maxp */
			clc.c_maxm = atoi(optarg);
			break;			
		default:
			printf("Unknown option %c\n", c);
			rv = -1;
			usage();
			goto bail;
		}
	}
	if (!clc.c_fname) {
		printf("Task set file required\n");
		rv = -1;
		usage();
		goto bail;
	}
	if (CONFIG_TRUE != config_read_file(&cfg, clc.c_fname)) {
		printf("Unable to read configuration file: %s\n",
		       clc.c_fname);
		printf("%s:%i %s\n", config_error_file(&cfg),
		       config_error_line(&cfg),
		       config_error_text(&cfg));
		rv = -1;
		goto bail;
	}
	if (clc.c_maxm <= 0) {
		fprintf(stderr, "Warning: maximum number of threads --maxm <= 0"
			" no divisions\n");
		goto bail;
	}
	/* 
	 * Must follow the reading of the original file, so that it may be
	 * overwritten
	 */
	if (clc.c_oname) {
		ofile = fopen(clc.c_oname, "w");
		if (!ofile) {
			printf("Unable to open %s for writing\n", clc.c_oname);
			ofile = stdout;
			goto bail;
		}
	}
	/*
	 * Configuration file parsed fully, let's go. 
	 */
	ts = ts_alloc();
	if (!ts_config_process(&cfg, ts)) {
		printf("Unable to process configuration file\n");
		rv = -1;
		goto bail;
	}
	config_destroy(&cfg);

	if (!check_ts(ts)) {
		printf("Task set %s is malformed, aborting!\n", clc.c_fname);
		rv = -1;
		goto bail;
	}

	gsl_rng *r = gsl_rng_alloc(gsl_rng_default);
	task_set_t *divided = ts_divide_set(ts, clc.c_maxm);
	gsl_rng_free(r);
	
	if (!divided) {
		printf("Could not divide task set %s\n", clc.c_fname);
		rv = -1;
		goto bail;
	}
	fprintf(ofile, "# Original task set file: %s\n", clc.c_fname);

	
	config_init(&cfg);
	/* Convert the task set to the config */
	ts_config_dump(&cfg, divided);
	/* Write the result */
	config_write(&cfg, ofile);
	ts_destroy(divided);
bail:
	ts_destroy(ts);
	config_destroy(&cfg);
	if (clc.c_oname) {
		free(clc.c_oname);
	}
	if (ofile != stdout) {
		fclose(ofile);
	}
	return rv;
}


#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "taskset-config.h"
#include "taskset-create.h"
#include "ex_uunifast.h"

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_lname;
	char* c_oname;
	char* c_fname;	
} clc;

enum {
      ARG_MINM = CHAR_MAX + 1,
      ARG_MAXM,
};

static const char* short_options = "hl:o:s:v";
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"log", required_argument, 0, 's'},
    {"output", required_argument, 0, 'o'},
    {"task-set", required_argument, 0, 's'},
    {"verbose", no_argument, &clc.c_verbose, 1},
    {0, 0, 0, 0}
};

void
usage() {
	printf("ts-merge: Merges a task to a single threaded WCET\n");
	printf("Usage: ts-merge [OPTIONS] -s <FILE> \n");
	printf("OPTIONS:\n");
	printf("\t--help/-h\t\tThis message\n");
	printf("\t--log/-l <FILE>\t\tAuditible log file\n");
	printf("\t--output/-o <FILE>\tOutput file of new task set\n");
	printf("\t--task-set/-s <FILE>\tTask set configuration file\n"); 	
	printf("\t--verbose/-v\t\tEnables verbose output\n");
	printf("\nOPERATION:\n");
	printf("\tTasks will be converted to a single threaded version with maximum WCET\n");
	printf("\nREQUIREMENTS:\n");
	printf("\tEach task must have a WCET for the maximum number of threads\n");
	printf("\nEXAMPLES:\n");
	printf("\\tMerges tasks to their maximum WCET value with 1 thread\n");
	printf("\t> ts-merge -s taskset.ts\n\n");
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
		uint32_t m = t->t_threads;
		if (m <= 0) {
			fprintf(stderr, "Error: task %s has %u threads\n", t->t_name, m);
		}
		uint32_t wcet = t->wcet(m);
		if (wcet <= 0) {
			fprintf(stderr,
				"Error: task %s has %u threads and a WCET"
				" for %u threads of %u", t->t_name,
				m, m, wcet);
			return 0;
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
		default:
			printf("Unknown option %c\n", c);
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

	task_set_t *merged = ts_merge(ts);
	if (!merged) {
		printf("Could not merge task set %s\n", clc.c_fname);
		rv = -1;
		goto bail;
	}
	fprintf(ofile, "# Original task set file: %s\n", clc.c_fname);
	
	config_init(&cfg);
	/* Convert the task set to the config */
	ts_config_dump(&cfg, merged);
	/* Write the result */
	config_write(&cfg, ofile);
	ts_destroy(merged);
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


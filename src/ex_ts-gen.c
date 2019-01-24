#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "taskset-config.h"
#include "taskset-create.h"
#include "ex_uunifast.h"

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	int c_tasks;
	char* c_lname;
	char* c_oname;
	uint32_t c_minp;
	uint32_t c_maxp;	
	uint32_t c_minm;
	uint32_t c_maxm;
	uint32_t c_totalm;
} clc;

enum {
      ARG_MINP = CHAR_MAX + 1,
      ARG_MAXP,
      ARG_MINM,
      ARG_MAXM,
      ARG_TOTM
};

static const char* short_options = "hl:n:o:v";
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"log", required_argument, 0, 's'},
    {"minp", required_argument, 0, ARG_MINP},
    {"maxp", required_argument, 0, ARG_MAXP},
    {"minm", required_argument, 0, ARG_MINM},
    {"maxm", required_argument, 0, ARG_MAXM},
    {"ntasks", required_argument, &clc.c_tasks, 1},
    {"output", required_argument, 0, 'o'},
    {"totalm", required_argument, 0, ARG_TOTM},    
    {"verbose", no_argument, &clc.c_verbose, 1},
    {0, 0, 0, 0}
};

void
usage() {
	printf("Task Set Generator: Creates an empty task set file that can be");
	printf(" updated by: \n\t");
	printf("uunnifast, ts-deadlines, and ts-threads.\n");
	printf("Usage: ts-gen [OPTIONS] -o <FILE> -n <INT> ||\n");
	printf("       ts-gen [OPTIONS] -o <FILE> --totalm <INT> --minm <INT>");
	printf(" --maxm <INT>\n");
	printf("OPTIONS:\n");
	printf("\t--help/-h\t\tThis message\n");
	printf("\t--log/-l <FILE>\t\tAuditible log file\n");
	printf("\t--minp <INT>\t\tMinimum period value of any task\n");
	printf("\t--maxp <INT>\t\tMaximum period value of any task\n");
	printf("\t--minm <INT>\t\tMinimum threads per task\n");
	printf("\t--maxm <INT>\t\tMaximum threads per task\n");
	printf("\t--ntasks/-n <INT>\tThe number of tasks\n");
	printf("\t--output/-o <FILE>\tOutput file of new task set\n");
	printf("\t--totalm <INT>\t\tTotal number of threads in the set\n");
	printf("\t--verbose/-v\t\tEnables verbose output\n");
	printf("\nRANGES:\n");
	printf("\tFor the minimum and maximum period values, if no minimum value");
	printf(" is provided\n\tthe default is zero. If no maximum value is given");
	printf(" the period of all tasks is\n\tzero\n\n");
	printf("\tFor the minimum and maximum threads per task, if no minimum value");
	printf(" is provided\n\tthe default is 1. If no maximum value is given");
	printf(" the number threads per tasks is\n\tzero\n");
	printf("\nEXAMPLES:\n");
	printf("\t10 tasks with 0 period, deadline, threads and wcet:\n");
	printf("\t> ts-gen -n 10\n\n");
	printf("\t10 tasks outputting a task set file:\n");
	printf("\t> ts-gen -n 10 -o ten-tasks.ts\n\n");
	printf("\t200 tasks with periods [10,100] uniformly distributed\n");
	printf("\t> ts-gen -n 200 --minp 10 --maxp 100\n\n");
	printf("\t20 tasks with periods [0,15]\n");
	printf("\t> ts-gen -n 20 --maxp 15\t# --minp 0 \n\n");
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
	
	clc.c_minm = 1;
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
		case 'n':
			clc.c_tasks = atoi(optarg);
			break;
		case 'v':
			clc.c_verbose = 1;
			break;
		case ARG_MINP: /* minp */
			clc.c_minp = atoi(optarg);
			break;
		case ARG_MAXP: /* maxp */
			clc.c_maxp = atoi(optarg);
			break;			
		case ARG_MINM: 
			clc.c_minm = atoi(optarg);
			break;
		case ARG_MAXM: 
			clc.c_maxm = atoi(optarg);
			break;			
		case ARG_TOTM: 
			clc.c_totalm = atoi(optarg);
			break;			
		default:
			printf("Unknown option %c\n", c);
			usage();
			goto bail;
		}
	}

	if (clc.c_oname) {
		ofile = fopen(clc.c_oname, "w");
		if (!ofile) {
			printf("Unable to open %s for writing\n", clc.c_oname);
			ofile = stdout;
			rv = -1;
			goto bail;
		}
	}

	if ((clc.c_tasks > 0) && (clc.c_maxm > 0)) {
		printf("Only one of -n or --maxp is permitted\n");
		rv = -1;
		usage();
		goto bail;
	}
	if (clc.c_maxm > 0 && clc.c_totalm <= 0) {
		printf("--totalm is required with --maxm\n");
		rv = -1;
		usage();
		goto bail;
	}

	/* Allocate the bare tasks */
	ts = ts_alloc();

	/* Initialize a random source */
	gsl_rng *r = gsl_rng_alloc(gsl_rng_default);
	if (clc.c_totalm > 0) {
		/* By thread count */
		tsc_add_by_thread_count(ts, r, clc.c_totalm, clc.c_minm, clc.c_maxm);
	} else {
		/* By task count */
		tsc_bare_addn(ts, clc.c_tasks);
	}
	
	/* Do they have periods? */
	if (clc.c_maxp > 0) {
		tsc_set_periods(ts, r, clc.c_minp, clc.c_maxp);
	}
	gsl_rng_free(r);
	
	/* Convert the task set to the config */
	ts_config_dump(&cfg, ts);
	/* Write the result */
	config_write(&cfg, ofile);
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


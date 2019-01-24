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
	float c_minf;
	float c_maxf;	
} clc;

enum {
      ARG_MINF = CHAR_MAX + 1,
      ARG_MAXF,
};

static const char* short_options = "hl:o:s:v";
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"log", required_argument, 0, 's'},
    {"minf", required_argument, 0, ARG_MINF},
    {"maxf", required_argument, 0, ARG_MAXF},
    {"output", required_argument, 0, 'o'},
    {"task-set", required_argument, 0, 's'},
    {"verbose", no_argument, &clc.c_verbose, 1},
    {0, 0, 0, 0}
};

void
usage() {
	printf("ts-gf: Task Set Growth Factor adjustment\n");
	printf("Usage: ts-gf [OPTIONS] -s <FILE> --minf <FLOAT> --maxf <FLOAT>\n");
	printf("OPTIONS:\n");
	printf("\t--help/-h\t\tThis message\n");
	printf("\t--log/-l <FILE>\t\tAuditible log file\n");
	printf("\t--minf <FLOAT>\t\tMinimum growth factor value of any task\n");
	printf("\t--maxf <FLOAT>\t\tMaximum frowth factor value of any task\n");
	printf("\t--output/-o <FILE>\tOutput file of new task set\n");
	printf("\t--task-set/-s <FILE>\tTask set configuration file\n"); 	
	printf("\t--verbose/-v\t\tEnables verbose output\n");
	printf("\nRANGES:\n");
	printf("\tA minimum and maximum growth factor must be provided in (0,1)");
	printf(" exclusive.\n\tEach task");
	printf(" has WCET values assigned by the method described in: \n");
	printf("\t    \"Non-Preemptive Multitask BUNDLE (working title)\"\n");
	printf("\t\t-- Tessler & Fisher, TBD\n");
	printf("\nREQUIREMENTS:\n");
	printf("\tAll tasks in the set must have at least one thread per job and");
	printf(" a non-zero\n\tWCET for the total number of threads\n"); 
	printf("\nEXAMPLES:\n");
	printf("\tAssign WCET values with minimum .1 and maximum .9\n");
	printf("\t> ts-gf -s taskset.ts --minf .1 --maxf .9\n\n");
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
			return 0;
		}
		uint32_t wcet = t->wcet(m);
		if (wcet <= 0) {
			fprintf(stderr, "Error: task %s has %u threads and a WCET"
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
		case ARG_MINF: /* minp */
			clc.c_minf = atof(optarg);
			break;
		case ARG_MAXF: /* maxp */
			clc.c_maxf = atof(optarg);
			break;			
		default:
			printf("Unknown option %c\n", c);
			usage();
			goto bail;
		}
	}
	if (clc.c_minf <= 0) {
		printf("0 < --minf < --maxf required\n");
		rv = -1;
		usage();
		goto bail;
	}
	if (clc.c_maxf <= 0 ||
	    clc.c_maxf < clc.c_minf) {
		printf("0 < --minf < --maxf required\n");
		rv = -1;
		usage();
		goto bail;
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

	fprintf(ofile, "# Original task set file: %s\n", clc.c_fname);

	gsl_rng *r = gsl_rng_alloc(gsl_rng_default);
	int succ = tsc_set_wcet_gf(ts, r, clc.c_minf, clc.c_maxf);
	gsl_rng_free(r);
	
	if (!succ) {
		printf("Could not perform WCET assignment on the given task set\n");
		rv = -1;
		goto bail;
	}
	
	config_init(&cfg);
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


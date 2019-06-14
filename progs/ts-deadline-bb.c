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
	char* c_fname;
	char* c_oname;
	tint_t c_maxd;	
} clc;

enum {
      ARG_MAXD = CHAR_MAX + 1
};

static const char* short_options = "hl:o:s:v";
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"log", required_argument, 0, 's'},
    {"maxd", required_argument, 0, ARG_MAXD},
    {"output", required_argument, 0, 'o'},
    {"task-set", required_argument, 0, 's'},
    {"verbose", no_argument, &clc.c_verbose, 1},
    {0, 0, 0, 0}
};

void
usage() {
	printf("Deadline assignment matching the method described in:\n");
	printf("\"Limited Preemption EDF Scheduling of Sporadic Task Systems\"");
	printf(", Bertogna & Baruah 2010.\n\n");
	printf("Each task of the set must have a period, number of threads, and WCETs.");
	printf(" The WCET for\nthe maximum number of threads must also be the maximum");
	printf(" WCET for each task.\n\n");
	printf("Usage: ts-deadline-bb [OPTIONS] -s <FILE> --maxd <INT>\n");
	printf("OPTIONS:\n");
	printf("\t--help/-h\t\tThis message\n");
	printf("\t--log/-l <FILE>\t\tAuditible log file\n");
	printf("\t--maxd <INT>\t\tMaximum deadline value of any task\n");	
	printf("\t--output/-o <FILE>\tOutput file of new task set\n");
	printf("\t--task-set/-s <FILE>\tTask set configuration file\n");
	printf("\t--verbose/-v\t\tEnables verbose output\n");
	printf("\nRANGES:\n");
	printf("\tFor the minimum and maximum period values, if no minimum value");
	printf(" is provided\n\tthe default is zero. If no maximum value is given");
	printf(" the period of all tasks is\n\tzero\n");
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
		case ARG_MAXD: /* maxd */
			clc.c_maxd = atoi(optarg);
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

	if (clc.c_oname) {
		ofile = fopen(clc.c_oname, "w");
		if (!ofile) {
			printf("Unable to open %s for writing\n", clc.c_oname);
			ofile = stdout;
			goto bail;
		}
	}
	fprintf(ofile, "# Original task set file: %s\n", clc.c_fname);	

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

	/* Initialize a random source */
	gsl_rng *r = gsl_rng_alloc(gsl_rng_default);
	tsc_set_deadlines_min_halfp(ts, r, 1, clc.c_maxd);
	gsl_rng_free(r);
	
	/* Convert the task set to the config */
	config_init(&cfg);
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


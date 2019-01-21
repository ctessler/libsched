#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "uunifast.h"
#include "taskset-config.h"
#include "ex_uunifast.h"

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	int c_tasks;
	float c_util;
	char* c_fname;
	char* c_lname;
	char* c_oname;
} clc;
static const char* short_options = "hl:o:s:u:v";
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"log", required_argument, 0, 's'},
    {"output", required_argument, 0, 'o'},
    {"task-set", required_argument, 0, 's'},
    {"util", required_argument, 0, 'u'},
    {"verbose", no_argument, &clc.c_verbose, 1},
    {0, 0, 0, 0}
};

void
usage() {
	printf("Implementation of UUniFast from \"Biasing Effects in Schedulability");
	printf(" Measures\"\n");
	printf("    -- Bini & Buttazo, 2004\n\n");
	printf("Usage: uunifast [OPTIONS] -s <TASKS> -u <UTIL>\n");
	printf("OPTIONS:\n");
	printf("\t--help/-h\t\tThis message\n");
	printf("\t--log/-l <FILE>\t\tAuditible log file\n");
	printf("\t--output/-o <FILE>\tOutput file of new task set\n"); 
	printf("\t--task-set/-s <FILE>\tTask set configuration file\n"); 
	printf("\t--util/-u <FLOAT>\tTotal system utilization [0,1]\n");	
	printf("\t--verbose/-v\t\tEnables verbose output\n");
	printf("\nOPERATION:\n");
	printf("\tThis implementation of UUniFast, adapts existing task sets");
	printf(" described by\n");
	printf("\ta task set (.ts) file. Every task in the .ts file must contain");
	printf(" either a\n\tperiod or a worst case execution time value. When a");
	printf(" task has many WCET\n\tvalues, the WCET of the maximum number of");
	printf(" threads will be used.\n");
	printf("\n\nRANDOMNESS:\n");
	printf("\tDistribution of values depends on the GNU Scientific Library.");
	printf("\n\tThe random function and its seed are configurable at run time.\n");
	printf("\tThe preferred invocation of uunifast is:\n\n");
	printf("\t> GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%%s` uunifast ...\n");
	printf("\n\nEXAMPLES:\n");
	printf("\tUUniFast for a utilization of .75\n");
	printf("\t> uunifast -s ex/uunifast.ts -u .75\n\n");
	printf("\tUtilization of .5 new task set in point5.ts\n");
	printf("\t> uunifast -s ex/uunifast.ts -u .5 -o point5.ts\n\n");
	printf("\tUse \'better\' random parameters\n");
	printf("\t> GSL_RNG_TYPE=ranlxs2 GSL_RNG_SEED=`date +%%s` \\\n");
	printf("\t\tuunifast -s ex/uunifast.ts -u .5 -o point5.ts\n");
	printf("\n%s\n", exfile);
}

int
main(int argc, char** argv) {
	task_set_t *ts = NULL;
	FILE *ofile = stdout;
	config_t cfg;
	int rv = 0;

	/* Initializer for libconfig */
	config_init(&cfg);
	/*
	 * Initializer for the GNU Scientific Library for random numbers
	 * Suggested values for environment variables
	 *   GSL_RNG_TYPE=ranlxs2
	 *   GSL_RNG_SEED=`date +%s`
	 */
	gsl_rng_env_setup();
	while(1) {
		int opt_idx = 0;
		char c = getopt_long(argc, argv, short_options,
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
		case 'u':
			clc.c_util = atof(optarg);
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

	if ((clc.c_util <= 0) || (clc.c_util > 1)) {
		printf("A total system utilization [-u] in the range (0, 1]");
		printf(" is required\n");
		rv = -1;
		usage();
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
	 * Configuration file parsed fully, let's go. 
	 */
	ts = ts_alloc();
	if (!ts_config_process(&cfg, ts)) {
		printf("Unable to process configuration file\n");
		rv = -1;
		goto bail;
	}
	config_destroy(&cfg);
	
	/*
	 * Configuration file processed, time to calculate the chunks
	 */
	char *str;
	printf("Original Task Set:\n");
	str = ts_string(ts); printf("%s\n\n", str); free(str);

	gsl_rng *r = gsl_rng_alloc(gsl_rng_default);
	int error = uunifast(ts, clc.c_util, r, NULL);
	gsl_rng_free(r);
	
	if (error) {
		printf("Could not perform UUniFast on the given task set\n");
		rv = error;
		goto bail;
	}
	
	printf("Normalized Utilization Task Set:\n");
	str = ts_string(ts); printf("%s\n", str); free(str);
	printf("-------------------------------------------------\n");
	printf("Utilization: %.4f, T*: %lu\n", ts_util(ts), ts_star(ts));

	config_init(&cfg);
	ts_config_dump(&cfg, ts);

	config_write(&cfg, ofile);
bail:
	ts_destroy(ts);
	config_destroy(&cfg);
	if (clc.c_fname) {
		free(clc.c_fname);
	}
	if (clc.c_oname) {
		free(clc.c_oname);
	}
	if (ofile != stdout) {
		fclose(ofile);
	}
	return rv;
}


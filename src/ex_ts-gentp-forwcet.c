#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "taskset-config.h"
#include "taskset-create.h"
#include "taskset-mod.h"
#include "uunifast.h"

int generate(task_set_t *orig, gen_parms_t *parms, FILE *output);

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_lname;
	char* c_oname;
	char* c_pname;
	char* c_tsname;
	float_t c_util;		
	tint_t c_minm;
	tint_t c_maxm;
	tint_t c_totalm;
} clc;

enum {
      ARG_MINM = CHAR_MAX + 1,
      ARG_MAXM,
};

static const char* short_options = "M:U:hl:o:p:s:v";
static struct option long_options[] = {
    {"help",		no_argument, 		0, 'h'},
    {"log", 		required_argument, 	0, 'l'},
    {"output", 		required_argument, 	0, 'o'},
    {"param",		required_argument,	0, 'p'},
    {"min-tpj", 	required_argument, 	0, ARG_MINM},
    {"max-tpj", 	required_argument, 	0, ARG_MAXM},
    {"task-set",	required_argument,	0, 's'},
    {"total-threads",	required_argument, 	0, 'M'},
    {"util",		required_argument,	0, 'U'},
    {"verbose", 	no_argument, 		&clc.c_verbose, 1},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"ts-gentp: Task Set Generator with Task Set Parameters for Tasks with WCET values",
"Usage: ts-gentp-forwcet [OPTIONS]",
"OPTIONS:",
"	-h/-help		This message",
"	-l/-log <FILE>		Auditible log file",
"	-o/--output <FILE>	Output file",
"	-p/--param <FILE>	Input parameter file",
"	-s/--task-set		Task set with WCET values",
"	-v/--verbose		Verbose output",
"",
"TASK SET OPTIONS:",
"	-M/--total-threads	Total number of threads in the set",
"	-U/--util <FLOAT>	Task set utilization",
"",
"INDIVIDUAL TASK OPTIONS:",
"	--[min|max]-tpj		Min./max. threads per job",
"",
"OPERATION:",
"	ts-gent-forwcet creates a new task set based upon one that already has WCET",
"	values assigned. In the new task set, each task is assigned (in order):",
"	- a number of threads within [min-tpj, max-tpj]",
"	- a period based on the WCET of the number of threads assigned",
"	- a deadline within the range [max(WCET, period/2), max_period]",
"",
"	WCET values are seeded with WCET entries per task. If a task has two entries",
"	the first and second threads will match the supplied values. The third and",
"	subsequent threads will increase the WCET by the difference of the two previous",
"	WCET values (the delta). If a task has *ONE* entry, threads will not be added.",
"	Instead, multiple tasks with one thread will be added to the set with the same",
"	period and deadline.",
"",
"PARAMETER FILES:",
"	Parameter files provide the values of the command line options in",
"	a more convenient format. See the example of ex/gentp-forwcet.tp",
"",
"EXAMPLES:",
"	> ts-gentp-forwcet -M 25 --min-tpj=2 --max-tpj=8 -U .5 \\",
"		-s ex/bundlep.ts -o bundlep-25.ts",
"",
"	> ts-gentp-forwcet -s ex/bundlep.ts -p ex/gentp-forwcet.tp",
"",
};

void
usage() {
	for (int i = 0; i < sizeof(usagec) / sizeof(usagec[0]); i++) {
		printf("%s\n", usagec[i]);
	}
}

int
main(int argc, char** argv) {
	FILE *ofile = stdout;
	gen_parms_t parms; /* Final task set generation parameters */
	config_t tmplt_cfg, ts_cfg;
	int rv = -1; /* Assume failure */

	/* Initilialize the config object */
	config_init(&tmplt_cfg);
	config_init(&ts_cfg);

	/*
	 * Initializer for the GNU Scientific Library for random numbers
	 * Suggested values for environment variables
	 *   GSL_RNG_TYPE=ranlxs2
	 *   GSL_RNG_SEED=`date +%s`
	 */
	ges_stfu();
	/* Parse those arguments! */
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
		case 'p':
			clc.c_pname = strdup(optarg);
			break;
		case ARG_MINM: 
			clc.c_minm = atoi(optarg);
			break;
		case ARG_MAXM: 
			clc.c_maxm = atoi(optarg);
			break;			
		case 'M': 
			clc.c_totalm = atoi(optarg);
			break;
		case 's':
			clc.c_tsname = strdup(optarg);
			break;
		case 'U':
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
	memset(&parms, 0, sizeof(gen_parms_t));
	/* If there is a parameter file, read it first */
	if (clc.c_pname) {
		if (CONFIG_TRUE != config_read_file(&tmplt_cfg, clc.c_pname)) {
			printf("Unable to read parameter file: %s\n",
			       clc.c_pname);
			printf("%s:%i %s\n", config_error_file(&tmplt_cfg),
			       config_error_line(&tmplt_cfg),
			       config_error_text(&tmplt_cfg));
			goto bail;
		}
		if (!ts_parm_process(&tmplt_cfg, &parms)) {
			printf("Incorrect parameters in %s\n", clc.c_pname);
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
	if (!clc.c_tsname) {
		printf("Task Set (--task-set) required\n");
		goto bail;
	}
	if (CONFIG_TRUE != config_read_file(&ts_cfg, clc.c_tsname)) {
		printf("Unable to read task set file: %s\n",
		       clc.c_tsname);
		printf("%s:%i %s\n", config_error_file(&ts_cfg),
		       config_error_line(&ts_cfg),
		       config_error_text(&ts_cfg));
		goto bail;
	}
	task_set_t *ts = ts_alloc();	
	if (!ts_config_process(&ts_cfg, ts)) {
		printf("Unable to process task set configuration %s\n", clc.c_tsname);
		goto bail;
	}
	
	
	/* Check the provided command line options, set the parameters */
	if (clc.c_util > 0) {
		parms.gp_util = clc.c_util;
	}
	if (clc.c_minm > 0) {
		parms.gp_minm = clc.c_minm;
	}
	if (clc.c_maxm > 0) {
		parms.gp_maxm = clc.c_maxm;
	}
	if (clc.c_totalm > 0) {
		parms.gp_totalm = clc.c_totalm;
	}

	rv = generate(ts, &parms, ofile);
bail:
	ts_destroy(ts);
	config_destroy(&tmplt_cfg);
	config_destroy(&ts_cfg);
	if (clc.c_oname) {
		free(clc.c_oname);
	}
	if (clc.c_pname) {
		free(clc.c_pname);
	}
	if (clc.c_tsname) {
		free(clc.c_tsname);
	}
	if (ofile != stdout) {
		fclose(ofile);
	}
	return rv;
}

/**
 * Generates the task set from the given parameters and writes them to
 * output
 *
 * @param[in] parms the generation parameters
 * @param[out] output the file to write the output to
 *
 * @return zero upon success, less than zero otherwise
 */
int
generate(task_set_t *orig, gen_parms_t *parms, FILE *output) {
	task_set_t *ts = NULL;
	gsl_rng *r = NULL;
	int rv = -1, e;
	config_t cfg;
	char *str;
	
	/* Check the parameters first */
	if (parms->gp_util <= 0) {
		printf("Utilization %f <= 0\n", parms->gp_util);
		return -1;
	}
	if (parms->gp_totalm < parms->gp_minm) {
		printf("Total threads: %u < %u threads per job\n",
		    parms->gp_totalm, parms->gp_minm);
		return -1;
	}
	if (parms->gp_minm > parms->gp_maxm) {
		printf("Min threads per job: %u > %u max threads per job\n",
		    parms->gp_totalm, parms->gp_minm);
		return -1;
	}

	ts = ts_dup(orig);
	if (!ts) {
		printf("Could not allocate a task set\n");
		goto bail;
	}

	if ((parms->gp_minm * ts_count(ts)) > parms->gp_totalm) {
		printf("Total number of threads %u is smaller than min threads"
		       " per job %lu * %u tasks\n", parms->gp_totalm,
		       ts_count(ts), parms->gp_minm);
		goto bail;
	}

	/* Initialize a random source */
	r = gsl_rng_alloc(gsl_rng_default);

	/* ts-gen work */
	if (clc.c_verbose) {
		printf("Distributing the total thread count of M:");
		printf("%u, total per job m in [%lu, %lu]\n",
		       parms->gp_totalm, parms->gp_minm, parms->gp_maxm);
	}
	e = tsm_dist_threads(r, ts, parms->gp_totalm, parms->gp_minm, parms->gp_maxm);
	if (e) {
		printf("Unable to distribute threads to tasks)\n");
		goto bail;
	}

	e = tsm_uunifast_periods(r, ts, parms->gp_util, NULL);
	if (e) {
		printf("Unable to set utilization\n");
		goto bail;
	}

	e = tsm_set_deadlines(r, ts, NULL);
	if (e) {
		printf("Unable to set deadlines\n");
		goto bail;
	}

	e = tsm_force_concave(ts, NULL);
	if (e) {
		printf("Unable to force concavity\n");
		goto bail;
	}

	config_init(&cfg);
	/* Convert the task set to the config */
	ts_config_dump(&cfg, ts);

	/* Write the result */
	config_write(&cfg, output);
	config_destroy(&cfg);

	rv = 0;

	if (!ts_is_constrained(ts)) {
		printf("Produced an unconstrained deadline task set!\n");
		rv = -1;
	}
 bail:
	if (r) {
		gsl_rng_free(r);
	}
	if (ts) {
		ts_destroy(ts);
	}
	return rv;
}

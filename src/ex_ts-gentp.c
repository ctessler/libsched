#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "taskset-config.h"
#include "taskset-create.h"
#include "uunifast.h"

int generate(gen_parms_t *parms, FILE *output);

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_lname;
	char* c_oname;
	char* c_pname;
	float_t c_minf;
	float_t c_maxf;
	float_t c_util;		
	uint32_t c_mind;
	uint32_t c_maxd;	
	uint32_t c_minp;
	uint32_t c_maxp;	
	uint32_t c_minm;
	uint32_t c_maxm;
	uint32_t c_totalm;
} clc;

enum {
      ARG_MINP = CHAR_MAX + 1,
      ARG_MAXP,
      ARG_MIND,
      ARG_MAXD,      
      ARG_MINF,
      ARG_MAXF,      
      ARG_MINM,
      ARG_MAXM,
};

static const char* short_options = "M:U:hl:o:p:v";
static struct option long_options[] = {
    {"help",		no_argument, 		0, 'h'},
    {"log", 		required_argument, 	0, 'l'},
    {"min-deadline", 	required_argument, 	0, ARG_MIND},
    {"max-deadline", 	required_argument, 	0, ARG_MAXD},
    {"min-factor",	required_argument,	0, ARG_MINF},
    {"max-factor",	required_argument,	0, ARG_MAXF},    
    {"output", 		required_argument, 	0, 'o'},
    {"param",		required_argument,	0, 'p'},
    {"min-period", 	required_argument, 	0, ARG_MINP},
    {"max-period", 	required_argument, 	0, ARG_MAXP},
    {"min-tpj", 	required_argument, 	0, ARG_MINM},
    {"max-tpj", 	required_argument, 	0, ARG_MAXM},    
    {"total-threads",	required_argument, 	0, 'M'},
    {"util",		required_argument,	0, 'U'},
    {"verbose", 	no_argument, 		&clc.c_verbose, 1},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"ts-gentp: Task Set Generator with Task Set Parameters",
"Usage: ts-gentp [OPTIONS]",
"OPTIONS:",
"	-h/-help		This message",
"	-l/-log <FILE>		Auditible log file",
"	-o/--output <FILE>	Output file",
"	-p/--param <FILE>	Input parameter file",
"	-v/--verbose		Verbose output",
"",
"TASK SET OPTIONS:",
"	-M/--total-threads	Total number of threads in the set",
"	-U/--util <FLOAT>	Task set utilization",
"",
"INDIVIDUAL TASK OPTIONS:",
"	--[min|max]-period	Min./max. period",
"	--[min|max]-deadline	Min./max. relative deadline",
"	--[min|max]-tpj		Min./max. threads per job",
"	--[min|max]-factor	Min./max. growth factor",
"",
"OPERATION:",
"	ts-gentp is a shortcut tool for generating task sets. It is equivalent to",
"	running the following series of commands. Command line options to ts-gentp",
"	are preceded with a $ (such as --total-threads,	which becomes $total-threads).",
"",
"	> ts-gen --totalm $total-threads --minp $min-period --maxp $max-period \\",
"		-o $output",
"	> uunifast -u $util -s $name -o $output",
"	> ts-gf --minf $min-factor --maxf $max-factor -s $name -o $output",
"	> ts-deadline-bb --maxd $max-deadline -s $name -o $output",
"",
"PARAMETER FILES:",
"	Parameter files provide the values of the command line options in",
"	a more convenient format. See the example of ex/mthreads.tp",
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
	config_t cfg;
	int rv = -1; /* Assume failure */

	/* Initilialize the config object */
	config_init(&cfg);

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
		case ARG_MIND:
			clc.c_mind = atoi(optarg);
			break;
		case ARG_MAXD:
			clc.c_maxd = atoi(optarg);
			break;
		case ARG_MINF:
			clc.c_minf = atof(optarg);
			break;
		case ARG_MAXF:
			clc.c_maxf = atof(optarg);
			break;
		case 'o':
			clc.c_oname = strdup(optarg);
			break;
		case 'p':
			clc.c_pname = strdup(optarg);
			break;
		case ARG_MINP:
			clc.c_minp = atoi(optarg);
			break;
		case ARG_MAXP:
			clc.c_maxp = atoi(optarg);
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
		if (CONFIG_TRUE != config_read_file(&cfg, clc.c_pname)) {
			printf("Unable to read parameter file: %s\n",
			       clc.c_pname);
			printf("%s:%i %s\n", config_error_file(&cfg),
			       config_error_line(&cfg),
			       config_error_text(&cfg));
			goto bail;
		}
		if (!ts_parm_process(&cfg, &parms)) {
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
	/* Check the provided command line options, set the parameters */
	if (clc.c_minf > 0) {
		parms.gp_minf = clc.c_minf;
	}
	if (clc.c_maxf > 0) {
		parms.gp_maxf = clc.c_maxf;
	}
	if (clc.c_util > 0) {
		parms.gp_util = clc.c_util;
	}
	if (clc.c_mind > 0) {
		parms.gp_mind = clc.c_mind;
	}
	if (clc.c_maxd > 0) {
		parms.gp_maxd = clc.c_maxd;
	}
	if (clc.c_minp > 0) {
		parms.gp_minp = clc.c_minp;
	}
	if (clc.c_maxp > 0) {
		parms.gp_maxp = clc.c_maxp;
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

	rv = generate(&parms, ofile);
bail:
	config_destroy(&cfg);
	if (clc.c_oname) {
		free(clc.c_oname);
	}
	if (clc.c_pname) {
		free(clc.c_pname);
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
generate(gen_parms_t *parms, FILE *output) {
	task_set_t *ts = NULL;
	gsl_rng *r = NULL;
	int rv = -1, e;
	config_t cfg;
	
	/* Check the parameters first */
	if (parms->gp_minf > parms->gp_maxf) {
		printf("Min growth factor %f > %f max growth factor\n",
		    parms->gp_minf, parms->gp_maxf);
		return -1;
	}
	if (parms->gp_mind > parms->gp_maxd) {
		printf("Min deadline %u > %u max deadline\n",
		    parms->gp_mind, parms->gp_maxd);
		return -1;
	}
	if (parms->gp_minp > parms->gp_maxp) {
		printf("Min period %u > %u max period\n",
		    parms->gp_minp, parms->gp_maxp);
		return -1;
	}
	if (parms->gp_util < 0) {
		printf("Utilization %f < 0\n", parms->gp_util);
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
	if (parms->gp_maxp <= 0) {
		printf("A maximum period is required (--max-period) or max-period"
		    " in the parameter file\n" );
		return -1;
	}
	if (parms->gp_minm <= 0) {
		printf("Threads per job (--min-tpj) or min-tpj must be greater "
		    "than zero\n",  parms->gp_minm);
		return -1;
	}
	if (parms->gp_maxf <= 0) {
		printf("Maximum growth factor must be > 0 (--max-factor)\n");
		return -1;
	}
	if (parms->gp_maxd <= 0) {
		printf("Maximum relative deadline must be > 0 (--max-deadline)\n");
		return -1;
	}
		       

	ts = ts_alloc();
	if (!ts) {
		printf("Could not allocate a task set\n");
		goto bail;
	}

	/* Initialize a random source */
	r = gsl_rng_alloc(gsl_rng_default);

	/* ts-gen work */
	if (clc.c_verbose) {
		printf("Adding tasks by total thread count of M:");
		printf("%u, total per job m in [%u, %u]\n",
		       parms->gp_totalm, parms->gp_minm, parms->gp_maxm);
	}
	e = tsc_add_by_thread_count(ts, r, parms->gp_totalm, parms->gp_minm,
	    parms->gp_maxm);
	if (e < 0) {
		printf("Unable to add tasks to the set (ie. ts-gen)\n");
		goto bail;
	}

	if (clc.c_verbose) {
		printf("Assigning periods to tasks in the range [%u, %u]\n",
		       parms->gp_minp, parms->gp_maxp);
	}
	e = tsc_set_periods(ts, r, parms->gp_minp, parms->gp_maxp);
	if (!e) {
		printf("Unable to set periods of tasks (ie. ts-gen)\n");
		goto bail;
	}

	/* utilization of task sets, ie UUniFast (uunifast) */
	if (clc.c_verbose) {
		printf("Assigning utilizations (UUniFast) to tasks for U=%.3f\n",
		       parms->gp_util);
	}
	e = uunifast(ts, parms->gp_util, r, NULL);
	if (e) {
		printf("Could not perform UUNifast (ie. uunifast)\n");
		goto bail;
	}

	/* Assign WCETs by growth factor */
	if (clc.c_verbose) {
		printf("Assigning WCET values by growth factor F in [%.3f, %.3f]\n",
		       parms->gp_minf, parms->gp_maxf);
	}
	e = tsc_set_wcet_gf(ts, r, parms->gp_minf, parms->gp_maxf);
	if (!e) {
		printf("Could not perform WCET assignment (ie. ts-gf)\n");
		goto bail;
	}

	/* Assign deadlines */
	if (clc.c_verbose) {
		printf("Assigning relative deadlines in [%u, %u]\n",
		       parms->gp_mind, parms->gp_maxd);
	}
	e = tsc_set_deadlines_min_halfp(ts, r, parms->gp_mind, parms->gp_maxd);
	if (!e) {
		printf("Could not assign relative deadlines (ie. ts-deadline-bb)\n");
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
		printf("mind: %u maxd: %u\n", parms->gp_mind, parms->gp_maxd);
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

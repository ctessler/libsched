#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "dag-task.h"
#include "dag-collapse.h"
#include "taskset-create.h"

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_lname;
	char* c_oname;
	char* c_tname;
	int c_count;
} clc;

static const char* short_options = "hl:o:vt:bcd:";
static struct option long_options[] = {
    {"help",		no_argument, 		0, 'h'},
    {"log", 		required_argument, 	0, 'l'},
    {"output", 		required_argument, 	0, 'o'},
    {"verbose", 	no_argument, 		&clc.c_verbose, 1},
    {"count",		no_argument,		0, 'c'},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"dts-candidates: DAG Task Candidate List",
"Usage: dts-candidates -t <TASK FILE> [OPTIONS]",
"OPTIONS:",
"	-h/-help		This message",
"	-l/-log <FILE>		Auditible log file",
"	-o/--output <FILE>	Output file",
"	-v/--verbose		Verbose output",
"	-c/--count		Output the total count",
"",
"OPERATION:",
"	dts-candidates produces the list of candidates per object",
"",
"EXAMPLES:",
"	# Find all candidates for task dtask.dot",
"	> dts-candidates -t dtask.dot -o dtask.cands",
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
	gsl_rng *r = NULL;
	dtask_t *task = NULL;
	int rv = -1; /* Assume failure */

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
		case 'v':
			clc.c_verbose = 1;
			break;
		case 'c':
			clc.c_count = 1;
			break;
		default:
			printf("Unknown option %c\n", c);
			usage();
			goto bail;
		}
	}
	if (!argv[optind]) {
		fprintf(stderr, "<TASK FILE> required\n");
		usage();
		goto bail;
	}
	clc.c_tname = strdup(argv[optind]);

	if (clc.c_oname) {
		ofile = fopen(clc.c_oname, "w");
		if (!ofile) {
			fprintf(stderr, "Unable to open %s for writing\n",
			    clc.c_oname);
			ofile = stdout;
			goto bail;
		}
	}

	/* Read the task file */
	task = dtask_read_path(clc.c_tname);
	if (!task) {
		fprintf(stderr, "Unable to read file %s\n", clc.c_tname);
		goto bail;
	}

	int max = dtask_max_object(task);
	if (max < 0) {
		fprintf(stderr, "Task has no objects\n");
		goto bail;
	}

	fprintf(ofile, "#Object\t<CAND1> <CAND2> ...\n");
	for (tint_t i=0; i <= max; i++) {
		fprintf(ofile, "%ld\t", i);
		dnl_t *list = dnl_by_obj(task, i);
		dnl_elem_t *e;
		dnl_foreach(list, e) {
			fprintf(ofile, "%s ", e->dnl_node->dn_name);
		}
		fprintf(ofile, "\n");
		dnl_clear(list);
		free(list);
	}

	if (clc.c_count) {
		fprintf(ofile, "Total Candidates: %d\n",
			dtask_count_cand(task));
	}
	
	rv = 0;
bail:
	if (task) {
		dtask_free(task);
	}
	if (clc.c_oname) {
		free(clc.c_oname);
	}
	if (clc.c_tname) {
		free(clc.c_tname);
	}
	if (r) {
		gsl_rng_free(r);
	}
	if (ofile != stdout) {
		fclose(ofile);
	}
	return rv;
}

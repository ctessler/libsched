#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>


#include "dag-task.h"
#include "dag-collapse.h"
#include "dag-task-set.h"
#include "dtaskset-config.h"
#include "taskset-create.h"

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_lname;
	char* c_oname;
	char* c_tname;
} clc;

static const char* short_options = "hl:o:v";
static struct option long_options[] = {
    {"help",		no_argument, 		0, 'h'},
    {"log", 		required_argument, 	0, 'l'},
    {"output", 		required_argument, 	0, 'o'},
    {"verbose", 	no_argument, 		&clc.c_verbose, 1},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"dts-infeas: prints a DAG task set",
"Usage: dts-infeas <TASK FILE> [OPTIONS]",
"OPTIONS:",
"	-h/-help		This message",
"	-l/-log <FILE>		Auditible log file",
"	-o/--output <FILE>	Output file",
"	-v/--verbose		Verbose output",
"",
"OPERATION:",
"	dts-infeas determines if the individual task is infeasible",
"	Infeasibility is defined by:",
"		critical path length >= deadline",
"",
"EXAMPLES:"
"	# Determines if the task is infeasible",
"	> dts-infeas 1task.dts",
"	# $? will be 0 if infeasible",
};

static char* header();
static char* summary(dtask_elem_t *e);

void
usage() {
	for (int i = 0; i < sizeof(usagec) / sizeof(usagec[0]); i++) {
		printf("%s\n", usagec[i]);
	}
}

int
main(int argc, char** argv) {
	FILE *ofile = stdout;
	dtask_t *task = NULL;
	int rv = -1; /* Assume failure */

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
	dtask_update(task);
	int infeas = dtask_infeasible(task);
	if (infeas) {
		rv = 0;
	}

	if (clc.c_verbose) {
		if (infeas) {
			fprintf(ofile, "INFEASIBLE\n");
		} else {
			fprintf(ofile, "FEASIBLE\n");
		}
	}
	
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
	if (ofile != stdout) {
		fclose(ofile);
	}
	return rv;
}

static char*
header() {
	static char buff[256];	
	sprintf(buff, "%-31s %-19s %6s %6s %6s %6s %6s %6s %6s %6s",
		"# Path", "Name", "Nodes", "WLoad", "CPathL", "Period",
		"DLine","Util", "Cores", "Infeas");
	return buff;
}

static char*
summary(dtask_elem_t *e) {
	static char buff[256];

	sprintf(buff, "%-31s %-19s %6d %6ld %6ld %6ld %6ld %6.2f %6ld %6s",
		e->dts_path, e->dts_task->dt_name,
		agnnodes(e->dts_task->dt_graph),
		dtask_workload(e->dts_task),
		dtask_cpathlen(e->dts_task),
		e->dts_task->dt_period,
		e->dts_task->dt_deadline,
		dtask_util(e->dts_task),
		dtask_cores(e->dts_task),
		(dtask_infeasible(e->dts_task) ? "infeas" : "feas"));

	return buff;
}

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
	char* c_sname;
} clc;

static const char* short_options = "hl:o:vs:";
static struct option long_options[] = {
    {"help",		no_argument, 		0, 'h'},
    {"log", 		required_argument, 	0, 'l'},
    {"output", 		required_argument, 	0, 'o'},
    {"verbose", 	no_argument, 		&clc.c_verbose, 1},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"dts-print: prints a DAG task",
"Usage: dts-print <TASK FILE> [OPTIONS]",
"OPTIONS:",
"	-h/-help		This message",
"	-l/-log <FILE>		Auditible log file",
"	-o/--output <FILE>	Output file",
"	-v/--verbose		Verbose output",
"",
"OPERATION:",
"	dt-prints prints the task set",
"",
"EXAMPLES:"
"	# Print task set task_3.dot",
"	> dt-print task_3.dot",
};

static char* header();
static char* summary(dtask_t *task);

void
usage() {
	for (int i = 0; i < sizeof(usagec) / sizeof(usagec[0]); i++) {
		printf("%s\n", usagec[i]);
	}
}

int
main(int argc, char** argv) {
	FILE *ofile = stdout;
	FILE *ifile = NULL;
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
		case 's':
			clc.c_sname = strdup(optarg);
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

	if (optind < argc) {
		clc.c_sname = strdup(argv[optind]);
	}
	if (!clc.c_sname) {
		fprintf(stderr, "DAG task set file required\n");
		goto bail;
	}
	ifile = fopen(clc.c_sname, "r");
	if (!ifile) {
		fprintf(stderr, "Could not open file %s\n", clc.c_sname);
		goto bail;
	}
	if (clc.c_oname) {
		ofile = fopen(clc.c_oname, "w");
		if (!ofile) {
			fprintf(stderr, "Unable to open %s for writing\n",
			    clc.c_oname);
			ofile = stdout;
			goto bail;
		}
	}
	
	task = dtask_read(ifile);
	if (!task) {
		fprintf(stderr, "Unable to process file %s\n", clc.c_sname);
		goto bail;
	}

	fprintf(ofile, "%s\n", header());
	fprintf(ofile, "%s\n", summary(task));
	
	rv = 0;
bail:
	if (task) {
		dtask_free(task);
	}
	if (clc.c_oname) {
		free(clc.c_oname);
	}
	if (clc.c_sname) {
		free(clc.c_sname);
	}
	if (ofile != stdout) {
		fclose(ofile);
	}
	return rv;
}

static char*
header() {
	static char buff[256];
	sprintf(buff, "%-19s %6s %6s %6s %6s %6s %6s %35s %6s",
		"# Name", "Nodes", "WLoad", "CPathL", "Period", "DLine",
		"Util", "Cores", "Infeas");
	return buff;
}

static char*
summary(dtask_t *task) {
	static char buff[256];
	char minbuff[128];

	sprintf(minbuff, "(%ld - %ld)/(%ld - %ld) = %ld",
		dtask_workload(task),
		dtask_cpathlen(task),
		task->dt_deadline,
		dtask_cpathlen(task),
		dtask_cores(task));

	sprintf(buff, "%-19s %6d %6ld %6ld %6ld %6ld %6.2f %35s %6s",
		task->dt_name,
		agnnodes(task->dt_graph),
		dtask_workload(task),
		dtask_cpathlen(task),
		task->dt_period,
		task->dt_deadline,
		dtask_util(task),
		minbuff,
		(dtask_infeasible(task) ? "infeas" : "feas"));

	return buff;
}

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
    {"task",		required_argument,	0, 't'},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"dts-print: prints a DAG task set",
"Usage: dts-print <TASK FILE> [OPTIONS]",
"OPTIONS:",
"	-h/-help		This message",
"	-l/-log <FILE>		Auditible log file",
"	-o/--output <FILE>	Output file",
"	-v/--verbose		Verbose output",
"",
"OPERATION:",
"	dts-prints summarizes the task set",
"",
"EXAMPLES:"
"	# Print task set 1task.dts",
"	> dts-print 1task.dts",
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
	config_t cfg;
	dtask_set_t *dts = NULL;
	int rv = -1; /* Assume failure */

	/* Initilialize the config object */
	config_init(&cfg);

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
	char *dir = strdup(clc.c_sname);
	dir = dirname(dir);
	
	if (clc.c_oname) {
		ofile = fopen(clc.c_oname, "w");
		if (!ofile) {
			fprintf(stderr, "Unable to open %s for writing\n",
			    clc.c_oname);
			ofile = stdout;
			goto bail;
		}
	}

	if (CONFIG_TRUE != config_read_file(&cfg, clc.c_sname)) {
		printf("Unable to read configuration file: %s\n",
		       clc.c_sname);
		printf("%s:%i %s\n", config_error_file(&cfg),
		       config_error_line(&cfg),
		       config_error_text(&cfg));
		goto bail;
	}

	dts = dts_alloc();
	int succ = dts_config_process(&cfg, dir, dts);
	if (!succ) {
		printf("Unable to process configuration file: %s\n",
		       clc.c_sname);
		goto bail;
	}

	fprintf(ofile, "%s\n", header());
	dtask_elem_t *cursor;
	dts_foreach(dts, cursor) {
		fprintf(ofile, "%s\n", summary(cursor));
	}
	
	rv = 0;
bail:
	config_destroy(&cfg);
	if (dts) {
		dts_clear(dts);
		dts_free(dts);
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
	sprintf(buff, "%-60s %-19s %6s %6s %6s %6s %6s %6s %6s %6s",
		"# Path", "Name", "Nodes", "WLoad", "CPathL", "Period",
		"DLine","Util", "Cores", "Infeas");
	return buff;
}

static char*
summary(dtask_elem_t *e) {
	static char buff[256];

	sprintf(buff, "%-60s %-19s %6d %6ld %6ld %6ld %6ld %6.2f %6ld %6s",
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

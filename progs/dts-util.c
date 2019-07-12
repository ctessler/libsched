#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>

#include "dag-task.h"
#include "dag-task-set.h"
#include "dtaskset-config.h"
#include "dag-collapse.h"

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_lname;
	char* c_oname;
	char* c_fname;
	int c_list;	/** File is a task list */
	int c_ignore;	/** ignore beneficial test */
} clc;

static const char* short_options = "hl:o:vt:L";
static struct option long_options[] = {
    {"help",		no_argument, 		0, 'h'},
    {"log", 		required_argument, 	0, 'l'},
    {"output", 		required_argument, 	0, 'o'},
    {"verbose", 	no_argument, 		&clc.c_verbose, 1},
    {"list",		no_argument,		0, 'L'},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"dts-util: Calculates the utilization of a DAG task (or list)"
"Usage: dts-util [OPTIONS] <FILE>",
"OPTIONS:",
"	-h/-help		This message",
"	-l/-log <FILE>		Auditible log file",
"	-o/--output <FILE>	Output file",
"	-v/--verbose		Verbose output",
"	-I/--ignore		Ignore \"beneficial\" test",
"	-L/--list		<FILE> is a task list",
"",
"OPERATION:",
"	dts-util calculates the utilization of a task (or task list)",
"",
"EXAMPLES:"
"	# Utilization of a single task",
"	> dts-util dtask.dot",
"",
"	> Utilization of a task set",
"	> dts-util tasks.dts",
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
	FILE *ifile = NULL;
	dtask_t *task = NULL;
	dtask_set_t *dts = NULL;
	int rv = -1; /* Assume failure */
	config_t cfg;	
	
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
		case 'v':
			clc.c_verbose = 1;
			break;
		case 'L':
			clc.c_list = 1;
			break;
		default:
			printf("Unknown option %c\n", c);
			usage();
			goto bail;
		}
	}
	if (!argv[optind]) {
		fprintf(stderr, "<FILE> required\n");
		usage();
		goto bail;
	}
	clc.c_fname = strdup(argv[optind]);

	ifile = fopen(clc.c_fname, "r");
	if (!ifile) {
		fprintf(stderr, "Unable to open %s for reading\n",
			clc.c_fname);
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

	char *dir = strdup(clc.c_fname);
	dir = dirname(dir);
	dts = dts_alloc();
	if (clc.c_list) {
		if (CONFIG_TRUE != config_read_file(&cfg, clc.c_fname)) {
			printf("Unable to read configuration file: %s\n",
			       clc.c_fname);
			printf("%s:%i %s\n", config_error_file(&cfg),
			       config_error_line(&cfg),
			       config_error_text(&cfg));
			goto bail;
		}
		int succ = dts_config_process(&cfg, dir, dts);
		if (!succ) {
			printf("Unable to process configuration file: %s\n",
			       clc.c_fname);
			goto bail;
		}
	} else {
		/* Read the task file */
		task = dtask_read_path(clc.c_fname);
		if (!task) {
			fprintf(stderr, "Unable to read file %s\n",
				clc.c_fname);
			goto bail;
		}
		dtask_elem_t *e = dtse_alloc(task);
		dts_insert_head(dts, e);
	}

	printf("%.2f\n", dts_util(dts));

	
	rv = 0;
bail:
	if (task) {
		dtask_free(task);
	}
	if (clc.c_oname) {
		free(clc.c_oname);
	}
	if (clc.c_fname) {
		free(clc.c_fname);
	}
	if (ofile != stdout) {
		fclose(ofile);
	}
	if (ifile) {
		fclose(ifile);
	}
	return rv;
}

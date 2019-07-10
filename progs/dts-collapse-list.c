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
	char* c_list_name;
	int c_ignore; /** ignore beneficial test */
} clc;

static const char* short_options = "hl:o:vt:L:I";
static struct option long_options[] = {
    {"help",		no_argument, 		0, 'h'},
    {"log", 		required_argument, 	0, 'l'},
    {"output", 		required_argument, 	0, 'o'},
    {"verbose", 	no_argument, 		&clc.c_verbose, 1},
    {"ignore",		no_argument,		0, 'I'},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"dts-collapse-list: DAG Task Collapsing of a Candidate List"
"Usage: dts-collapse-list -L <LIST FILE> <TASK FILE> [OPTIONS]",
"OPTIONS:",
"	-h/-help		This message",
"	-l/-log <FILE>		Auditible log file",
"	-o/--output <FILE>	Output file",
"	-v/--verbose		Verbose output",
"	-I/--ignore		Ignore \"beneficial\" test",
"REQUIRED:",
"	-L/--list <FILE>	Collapse list, from dts-cand-order",
"",
"OPERATION:",
"	dts-collapse-list collapses nodes as a list given by",
"	dts-cand-order.",
"",
"EXAMPLES:"
"	# Collapse from cand.list in dtask.dot",
"	> dts-collapse-list -L cand.list dtask.dot",
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
	FILE *lfile = NULL;
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
		case 'L':
			clc.c_list_name = strdup(optarg);
			break;
		case 'I':
			clc.c_ignore = 1;
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

	if (!clc.c_list_name) {
		fprintf(stderr, "--list-file is a required option\n");
		usage();
		goto bail;
	}
	lfile = fopen(clc.c_list_name, "r");
	if (!lfile) {
		fprintf(stderr, "Unable to open %s for reading\n",
			clc.c_list_name);
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

	/* Read the task file */
	task = dtask_read_path(clc.c_tname);
	if (!task) {
		fprintf(stderr, "Unable to read file %s\n", clc.c_tname);
		goto bail;
	}

	char a_name[DT_NAMELEN], b_name[DT_NAMELEN];
	while (fscanf(lfile, "%s %s\n", a_name, b_name) != EOF) {
		dnode_t *a = dtask_name_match(task, a_name);
		dnode_t *b = dtask_name_match(task, b_name);

		if (a == NULL) {
			fprintf(stderr, "Could not find a matching node %s\n",
				a_name);
			goto bail;
		}
		if (b == NULL) {
			fprintf(stderr, "Could not find a matching node %s\n",
				b_name);
			goto bail;
		}
		if (!dag_can_collapse(a, b)) {
			fprintf(stderr, "Cannot collapse %s and %s, skipping\n",
				a->dn_name, b->dn_name);
			dnode_free(a);
			dnode_free(b);
			continue;
		}
		dtask_t *copy = dtask_copy(task);
		dnode_t *ca = dtask_name_match(copy, a_name);
		dnode_t *cb = dtask_name_match(copy, b_name);
		int beneficial = 1;
		if (!dag_collapse(ca, cb)) {
			fprintf(stderr, "Collapse of %s and %s failed\n",
				ca->dn_name, cb->dn_name);
			goto bail;
		}
		float_t pre_m = dtask_coresf(task);
		float_t post_m = dtask_coresf(copy);
		if (pre_m < post_m) {
			fprintf(stderr, "Collapse of %s and %s is not "
				"beneficial: m increases\n",
				ca->dn_name, cb->dn_name);
			beneficial = 0;
		}
		tint_t post_l = dtask_cpathlen(copy);
		if (post_l > task->dt_deadline) {
			fprintf(stderr, "Collapse of %s and %s is not "
				"beneficial: L > D\n",
				ca->dn_name, cb->dn_name);
			beneficial = 0;
		}
		dnode_free(ca);
		dnode_free(cb);
		dtask_free(copy);

		if (!beneficial && !clc.c_ignore) {
			goto next_iter;
		}
		
		if (!dag_collapse(a, b)) {
			fprintf(stderr, "Collapse of %s and %s failed\n",
				a->dn_name, b->dn_name);
			goto bail;
		}
	next_iter:
		dtask_update(task);
		dnode_free(a); a = NULL;
		dnode_free(b); b = NULL;
	}
	dtask_update(task);
	dtask_write(task, ofile);
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
	if (ofile != stdout) {
		fclose(ofile);
	}
	if (lfile) {
		fclose(lfile);
	}
	return rv;
}

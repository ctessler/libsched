#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "dag-task.h"
#include "taskset-create.h"

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_lname;
	char* c_oname;
	char* c_tname;
	tint_t c_nodes;
	float_t c_edgep;
} clc;

static const char* short_options = "hl:o:n:e:v";
static struct option long_options[] = {
    {"help",		no_argument, 		0, 'h'},
    {"log", 		required_argument, 	0, 'l'},
    {"name",		required_argument,	0, 'a'},
    {"nodes",		required_argument,	0, 'n'},
    {"edgep",		required_argument,	0, 'e'},
    {"output", 		required_argument, 	0, 'o'},
    {"verbose", 	no_argument, 		&clc.c_verbose, 1},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"dts-gen-nodes: DAG Task Generator for Nodes and Edges"
"Usage: dts-gen-nodes [OPTIONS]",
"OPTIONS:",
"	-h/-help		This message",
"	-l/-log <FILE>		Auditible log file",
"	-a/-name <STRING>	Task name",
"	-o/--output <FILE>	Output file",
"	-v/--verbose		Verbose output",
"REQUIRED:",
"	-n/--nodes <INT>	Number of nodes",
"	-e/--edgep <FLOAT>	Probability of an edge between nodes [0,1]",
"",
"OPERATION:",
"	dts-gen-nodes is the first stage in task set generation. It defines a DAG",
"	task in terms of the nodes and edges between nodes. No other properties of the",
"	task are assigned.",
"",
"EXAMPLES:"
"	# Generate a task with 20 nodes where the probability of an edge is 70%",
"	> dts-gen-nodes -n 20 -e 0.7",
"",
"	# Generate a task file for 15 nodes with an edge probability of 50%",
"	> dts-gen-nodes -n 15 -e 0.5 -o fifteen.dot",
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
		case 'a':
			clc.c_tname = strdup(optarg);
			break;
		case 'n': 
			clc.c_nodes = atoi(optarg);
			break;
		case 'e':
			clc.c_edgep = atof(optarg);
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

	if (clc.c_nodes <= 0) {
		fprintf(stderr, "--nodes is a required option\n");
		goto bail;
	}
	if (clc.c_edgep > 1 || clc.c_edgep <= 0) {
		fprintf(stderr, "--edgep is a required option (0,1]\n");
		goto bail;
	}
	if (clc.c_oname) {
		ofile = fopen(clc.c_oname, "w");
		if (!ofile) {
			fprintf(stderr, "Unable to open %s for writing\n", clc.c_oname);
			ofile = stdout;
			rv = -1;
			goto bail;
		}
	}
	if (!clc.c_tname) {
		clc.c_tname = calloc(1, DT_NAMELEN);
		snprintf(clc.c_tname, DT_NAMELEN, "Task{n=%ld,e=%0.2f}", clc.c_nodes,
			 clc.c_edgep);
	}
		
	char buff[DT_NAMELEN];

	/* Create the DAG Task */
	task = dtask_alloc(clc.c_tname);
	for (int i=0; i < clc.c_nodes; i++) {
		sprintf(buff, "n_%d", i);
		dnode_t *n = dnode_alloc(buff);
		dtask_insert(task, n);
		dnode_free(n);
	}

	/* Initialize a random source */
	r = gsl_rng_alloc(gsl_rng_default);

	/* The first node is reserved as the source node, the final nodes as the sink */
	for (int i=0; i < clc.c_nodes; i++) {
		sprintf(buff, "n_%d", i);
		dnode_t *a = dtask_name_search(task, buff);
		for (int j=i + 1; j < clc.c_nodes; j++) {
			double sel = tsc_get_scaled_dbl(r, 0, 1);
			if (sel < clc.c_edgep) {
				/* Time to add an edge */
				sprintf(buff, "n_%d", j);
				dnode_t *b = dtask_name_search(task, buff);
				dtask_insert_edge(task, a, b);
			}
		}
		dnode_free(a);
	}

	/* Source and sink assurance */
	sprintf(buff, "n_%d", 0);	
	dnode_t *source = dtask_name_search(task, buff);
	sprintf(buff, "n_%ld", (clc.c_nodes - 1));	
	dnode_t *sink = dtask_name_search(task, buff);
	
	for (int i=1; i < clc.c_nodes - 1; i++) {
		sprintf(buff, "n_%d", i);
		dnode_t *n = dtask_name_search(task, buff);
		/* If there are no incoming edges, make one from the source */
		if (dnode_indegree(n) == 0) {
			dtask_insert_edge(task, source, n);
		}
		/* If there are no outgoing edges, make one to the sink */
		if (dnode_outdegree(n) == 0) {
			dtask_insert_edge(task, n, sink);
		}
		dnode_free(n);
	}
	dnode_free(source);
	dnode_free(sink);

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
	if (r) {
		gsl_rng_free(r);
	}
	if (ofile != stdout) {
		fclose(ofile);
	}
	return rv;
}

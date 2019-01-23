#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "maxchunks.h"
#include "taskset-config.h"
#include "ex_tasks.h"
		   
/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_fname;
	int c_nonp;
} clc;

static const char* short_options = "hl:s:v";
static struct option long_options[] = {
    {"task-set", required_argument, 0, 's'},
    {"help", no_argument, 0, 'h'},
    {"nonp", no_argument, &clc.c_nonp, 1},
    {"verbose", no_argument, &clc.c_verbose, 1},
    {0, 0, 0, 0}
};

void
usage() {
	printf("Usage: max-chunks [OPTIONS] -s <TASKSET>\n");
	printf("OPTIONS:\n");
	printf("\t--help/-h\t\tThis message\n");
	printf("\t--log/-l <FILE>\t\tAuditible log file\n");
	printf("\t--nonp\t\t Nonpreemptive feasibility only if chunks >= WCET\n");
	printf("\t--task-set/-s <FILE>\tRequired file containing tasks\n");
	printf("\t--verbose/-v\t\tEnables verbose output\n");
	printf("\n%s\n", exfile);
}

int
main(int argc, char** argv) {
	config_t cfg;
	task_set_t *ts = NULL;
	int rv = 0;

	config_init(&cfg);
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
		case 'l':
			/* Needs to be implemented */
			printf("Log file not implemented\n");
			usage();
			goto bail;
		case 's':
			clc.c_fname = strdup(optarg);
			printf("Task set file: %s\n", clc.c_fname);
			break;
		}
	}
	if (!clc.c_fname) {
		printf("Task set file required\n");
		rv = -1;
		usage();
		goto bail;
	}

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
	/*
	 * Configuration file processed, time to calculate the chunks
	 */
	char *str;
	printf("Task Set:\n");
	str = ts_string(ts); printf("%s\n\n", str); free(str);
	int feas = max_chunks(ts);
	if (clc.c_nonp) {
		/* Non-preemptive check */
		feas = max_chunks_nonp(ts);
	}


	printf("After assigning non-preemptive chunks\n");
	str = ts_string(ts); printf("%s\n", str); free(str);
	printf("-------------------------------------------------\n");
	printf("Utilization: %.4f, T*: %lu, Feasible: ", ts_util(ts),
	    ts_star(ts));
	switch (feas) {
	case 0:
		printf("Yes\n");
		rv = 0;
		break;
	case 1:
		printf("No\n");
		rv = 1;
		break;
	case -1:
		printf("N/A (Poorly formed set)\n");
		rv = -1;
		break;
	}
bail:
	ts_destroy(ts);
	config_destroy(&cfg);
	if (clc.c_fname) {
		free(clc.c_fname);
	}
	return rv;
}


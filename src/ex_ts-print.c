#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "taskset-config.h"

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	int c_util;
	char* c_fname;	
} clc;


static const char* short_options = "hs:u";
static struct option long_options[] = {
    {"help",		no_argument, 0, 'h'},
    {"utilization",	no_argument, 0, 'u'},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"ts-print: Prints a task set file",
"",
"Usage: ts-print <FILE>",
"	-h/--help		This message",
"	-u/--utilization	Prints *only* the utilization",
""
};

void
usage() {
        for (int i = 0; i < sizeof(usagec) / sizeof(usagec[0]); i++) {
		printf("%s\n", usagec[i]);
	}
}

int
main(int argc, char** argv) {
	task_set_t *ts = NULL;
	char *str;
	config_t cfg;
	int rv = 0;

	/* Initilialize the config object */
	config_init(&cfg);
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
		case 's':
			clc.c_fname = strdup(optarg);
			break;
		case 'u':
			clc.c_util = 1;
			break;
		default:
			printf("Unknown option %c\n", c);
			usage();
			goto bail;
		}
	}
	if (optind < argc) {
		clc.c_fname = strdup(argv[optind]);
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
	if (clc.c_util) {
		printf("%.4f\n", ts_util(ts));
		goto bail;
	}
	
	str = ts_header(ts); printf("%s\n", str); free(str);	
	str = ts_string(ts); printf("%s\n", str); free(str);
	printf("-------------------------------------------------\n");
	printf("Utilization: %.4f, T*: %lu\n", ts_util(ts), ts_star(ts));
	
bail:
	ts_destroy(ts);
	config_destroy(&cfg);
	if (clc.c_fname) {
		free(clc.c_fname);
	}
	return rv;
}


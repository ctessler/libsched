#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_fname;
} clc;

static const char* short_options = "f:v";
static struct option long_options[] = {
    {"file", required_argument, 0, 'f'},
    {"verbose", no_argument, &clc.c_verbose, 1},
    {0, 0, 0, 0}
};

int
main(int argc, char** argv) {
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
		case 'f':
			clc.c_fname = strdup(optarg);
			printf("Configuration File: %s\n", clc.c_fname);
			break;
		}
	}
	if (!clc.c_fname) {
		printf("Configuration file required\n");
		return(-1);
	}

	config_t cfg;
	config_init(&cfg);
	config_destroy(&cfg);

	free(clc.c_fname);
	return 0;
}

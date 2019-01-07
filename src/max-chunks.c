#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "maxchunks.h"
#include "max-chunks.h"
/**
 * Processes the parsed configuration file into a set of tasks.
 *
 * @param[in] cfg the in memory configuration file
 * @param[in|out] ts the task set resulting from processing cfg
 *
 * @return non-zero upon success, zero otherwise
 */
int config_process(config_t *cfg, task_set_t *ts);
		   
/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_fname;
} clc;

static const char* short_options = "ht:v";
static struct option long_options[] = {
    {"tasks", required_argument, 0, 't'},
    {"help", no_argument, 0, 'h'},    
    {"verbose", no_argument, &clc.c_verbose, 1},
    {0, 0, 0, 0}
};

void
usage() {
	printf("Usage: max-chunks [OPTIONS] -t <TASKSET>\n");
	printf("OPTIONS:\n");
	printf("\t--help/-h\t\tThis message\n");
	printf("\t--log/-l <FILE>\t\tAuditible log file\n");
	printf("\t--tasks/-t <FILE>\tRequired file containing tasks\n");
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
		case 't':
			clc.c_fname = strdup(optarg);
			printf("Configuration File: %s\n", clc.c_fname);
			break;
		}
	}
	if (!clc.c_fname) {
		printf("Configuration file required\n");
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
	if (!config_process(&cfg, ts)) {
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

	printf("After assigning non-preemptive chunks\n");
	str = ts_string(ts); printf("%s\n", str); free(str);
	printf("-------------------------------------------------\n");
	printf("Utilization: %.4f, T*: %lu, Feasible: ", ts_util(ts),
	    ts_star(ts));
	switch (feas) {
	case 0:
		printf("Yes\n");
		break;
	case 1:
		printf("No\n");
		break;
	case -1:
		printf("N/A (Poorly formed set)\n");
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

int
config_process(config_t *cfg, task_set_t *ts) {
	config_setting_t *setting;

	setting = config_lookup(cfg, "tasks");
	if (setting == NULL) {
		printf("No tasks in the configuration file\n");
		return 0;
	}

	int ntasks = config_setting_length(setting);
	for (int i = 0; i < ntasks; i++) {
		config_setting_t *cs_task =
		    config_setting_get_elem(setting, i);
		uint32_t cs_period, cs_deadline, cs_threads;
		const char *cs_name;
		if (!config_setting_lookup_string(
		    cs_task, "name", &cs_name)) {
			printf("No name for task %i\n", (i+1));
			return 0;
		}

		if (!config_setting_lookup_int(
		    cs_task, "period", &cs_period)) {
			printf("No period for task %s\n", cs_name);
			return 0;
		}

		if (!config_setting_lookup_int(
		    cs_task, "deadline", &cs_deadline)) {
			printf("No period for task %s\n", cs_name);
			return 0;
		}

		if (!config_setting_lookup_int(
		    cs_task, "threads", &cs_threads)) {
			printf("No thread count for task %s\n",
			    cs_name);
			return 0;
		}

		config_setting_t *cs_wcet =
		    config_setting_get_member(cs_task, "wcet");
		if (cs_threads != config_setting_length(cs_wcet)) {
			printf("Expected %i WCETs found %i\n",
			       cs_threads,
			       config_setting_length(cs_wcet));
			return 0;
		}
		task_t *task = task_alloc(cs_period, cs_deadline,
		    cs_threads);

		strncpy(task->t_name, cs_name, TASK_NAMELEN);
		for (int j = 1; j <= cs_threads; j++) {
			uint32_t wcet =
			    config_setting_get_int_elem(cs_wcet, (j-1));
			task->wcet(j) = wcet;
		}

		ts_add(ts, task);
	}
	
	return 1;
}

#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>


#include "dag-collapse.h"
#include "dag-task-set.h"
#include "dtaskset-config.h"
#include "taskset-create.h"
#include "dag-task-to-task.h"
#include "tpj.h"

/**
 * global command line configuration
 */
static struct {
	int c_verbose;
	char* c_lname;
	char* c_oname;
	char* c_sname;
	int c_m;
	int c_nonp;
	int c_p;
	int c_best_fit;
} clc;


static const char* short_options = "hl:o:vm:pPe";
static struct option long_options[] = {
    {"help",		no_argument, 		0, 'h'},
    {"log", 		required_argument, 	0, 'l'},
    {"output", 		required_argument, 	0, 'o'},
    {"verbose", 	no_argument, 		&clc.c_verbose, 1},
    {"task",		required_argument,	0, 't'},
    {"cores",		required_argument,	0, 'm'},
    {"non-preemptive",	no_argument,		0, 'P'},
    {"preemptive",	no_argument,		0, 'p'},
    {"best-fit",	no_argument,		0, 'e'},
    {0, 0, 0, 0}
};

static const char *usagec[] = {
"dts-sched: Determines if a DAG task is schedulable",
"Usage: dts-sched <TASK SET FILE> [OPTIONS]",
"OPTIONS:",
"	-h/-help		This message",
"	-l/-log <FILE>		Auditible log file",
"	-o/--output <FILE>	Output file",
"	-v/--verbose		Verbose output",
"",
"REQUIRED OPTIONS:",
"	-m/--cores <INT>	Number of cores",
"",
"LOW UTILIZATION SCHEDULING:",
"	-P/--non-preemptive	Non-Preemptive EDF (default)",
"	-p/--preemptive		Preemptive EDF",
"",
"LOW UTILIZATION PARTITIONING:",
"	-e/--best-fit		Best Fit Core Assignment (default)",
"",
"OPERATION:",
"	dts-sched determines if a DAG task is schedulable. High utilization",
"	tasks are given dedicated cores. Low utilization tasks are assigned",
"	to cores given a partitioning heuristic, and evaluated using a",
"	scheduling algorithm per core."
"",
"EXAMPLES:"
"	# Determine if 4dtasks.dts is schedulable on 16 cores"
"	> dts-sched -m 16 4dtsk.dts",
};

static char* header();
static char* summary(int ntasks, int infeas, int sched, int m_high,
    int m_low, float_t util);
static int low_sched(int m_low, dtask_set_t* low);

void
usage() {
	for (int i = 0; i < sizeof(usagec) / sizeof(usagec[0]); i++) {
		printf("%s\n", usagec[i]);
	}
}

int
main(int argc, char** argv) {
	clc.c_nonp = 1;
	clc.c_best_fit = 1;

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
		case 'm':
			clc.c_m = atoi(optarg);
			break;
		case 'p':
			clc.c_p = 1;
			clc.c_nonp = 0;
			break;
		case 'e':
			clc.c_best_fit = 1;
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

	if (clc.c_m <= 0) {
		fprintf(stderr, "Number of --cores requird\n");
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


	int m_high = 0, m_low = 0, sched = 0, infeas = 0, ntasks=0;
	float_t util = 0;
	dtask_elem_t *cursor;
	dtask_set_t *low = dts_alloc();
	
	dts_foreach(dts, cursor) {
		dtask_t *task = cursor->dts_task;
		/* Remove the task from the original task set */
		cursor->dts_task = NULL;
		dtask_elem_t *elem = dtse_alloc(task);
		
		int cores;
		ntasks++;
		if (dtask_infeasible(task)) {
			infeas = 1;
			continue;
		}
		util += dtask_util(task);
		cores = dtask_cores(task);
		if (cores > 1) {
			m_high += cores;
		} else {
			dts_insert_head(low, elem);
		}
	}
	dts_clear(dts);
	dts_free(dts);
	dts = NULL;
	
	if (infeas) {
		goto done;
	}

	m_low = clc.c_m - m_high;
	if (m_low < 0) {
		m_low = 0;
	}
	if (low_sched(m_low, low)) {
		sched = 1;
	}

done:
	dts_clear(low); dts_free(low);

	fprintf(ofile, "%s\n", header());
	fprintf(ofile, "%s\n",
		summary(ntasks, infeas, sched, m_high, m_low, util));
	
	rv = 0;
	if (!sched) {
		rv = -1;
	}
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

static int
best_fit(int m_low, task_set_t** ts, task_t *task) {
	int cur = -1;
	float_t best = 1.1;
	float_t nec = 1 - task_util(task);
	for (int i=0; i < m_low; i++) {
		float_t remain = 1 - ts_util(ts[i]);
		if (remain > nec) {
			/* The task fits */
			if (remain < best) {
				/* it fits better */
				cur = i;
				best = remain;
			}
		}
	}
	return cur;
}


static int
low_sched(int m_low, dtask_set_t* low) {
	int rv = 0;
	if (m_low <= 0) {
		return rv;
	}

	task_set_t* parts[m_low];
	for (int i=0; i < m_low; i++) {
		parts[i] = ts_alloc();
	}

	dtask_elem_t *e;
	dts_foreach(low, e) {
		task_t *task = dt_to_t(e->dts_task);
		/** Best fit */
		int idx = best_fit(m_low, parts, task);
		if (idx < 0) {
			goto done;
		}
		ts_add(parts[idx], task);
	}

	for (int i=0; i < m_low; i++) {
		if (tpj(parts[i], NULL) != 0) {
			goto done;
		}
	}
	
	rv = 1;

done:
	for (int i=0; i < m_low; i++) {
		ts_destroy(parts[i]);
	}
	return rv;
}

static char*
header() {
	static char buff[256];	
	sprintf(buff, "%-7s %-6s %-5s %-6s %-5s %6s",
		"# tasks", "infeas", "sched", "m_high", "m_low", "util");
	return buff;
}

static char*
summary(int ntasks, int infeas, int sched, int m_high,
	int m_low, float_t util) {
	static char buff[256];

	sprintf(buff, "%7d %6s %5s %6d %5d %6.2f",
		ntasks, (infeas ? "yes" : "no"), (sched ? "yes" : "no"),
		m_high, m_low, util);
	return buff;
}

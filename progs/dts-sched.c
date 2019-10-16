#include <stdio.h>
#include <libconfig.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <signal.h>


#include "dag-collapse.h"
#include "dag-task-set.h"
#include "dtaskset-config.h"
#include "taskset-create.h"
#include "dag-task-to-task.h"
#include "tpj.h"
#include "maxchunks.h"

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
	int c_worst_fit;
	int c_timeout;
} clc;


static const char* short_options = "hl:o:vm:pPewt:";
static struct option long_options[] = {
    {"help",		no_argument, 		0, 'h'},
    {"log", 		required_argument, 	0, 'l'},
    {"output", 		required_argument, 	0, 'o'},
    {"verbose", 	no_argument, 		&clc.c_verbose, 1},
    {"timeout",		required_argument,	0, 't'},
    {"cores",		required_argument,	0, 'm'},
    {"non-preemptive",	no_argument,		0, 'P'},
    {"preemptive",	no_argument,		0, 'p'},
    {"best-fit",	no_argument,		0, 'e'},
    {"worst-fit",	no_argument,		0, 'w'},
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
"	-t/--timeout <MINUTES>	Execution time cap (default:unset)",
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
"	-w/--worst-fit		Worst Fit Core Assignment",
"",
"OPERATION:",
"	dts-sched determines if a DAG task is schedulable. High utilization",
"	tasks are given dedicated cores. Low utilization tasks are assigned",
"	to cores given a partitioning heuristic, and evaluated using a",
"	scheduling algorithm per core."
"",
"	If a -t <MIN> option is given, the task set will be deemed",
"	unschedulable if the schedulability test taks MIN or more minutes",
"	to complete.",
"",
"EXAMPLES:"
"	# Determine if 4dtasks.dts is schedulable on 16 cores"
"	> dts-sched -m 16 4dtsk.dts",
};

static char* header();
static char* summary(int ntasks, int infeas, int sched, int m_high,
    int m_low, float_t util);
static int low_sched(int m_low, dtask_set_t* low);
static void set_timeout(int minutes, int ntasks, int mhigh, int mlow,
			float util, FILE *ofile);

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
			clc.c_worst_fit = 0;
			break;
		case 'w':
			clc.c_best_fit = 0;
			clc.c_worst_fit = 1;
			break;
		case 't':
			clc.c_timeout = atoi(optarg);
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

	util = dts_util(dts);
	dts_foreach(dts, cursor) {
		dtask_t *task = cursor->dts_task;
		/* Remove the task from the original task set */
		dtask_elem_t *elem = dtse_alloc(task);
		
		int cores;
		ntasks++;
		if (dtask_infeasible(task)) {
			infeas = 1;
			continue;
		}
		float_t task_util = dtask_util(task);
		cores = dtask_cores(task);
		if (task_util > 1) {
			m_high += cores;
		} else {
			dts_insert_head(low, elem);
		}
	}

	dts_free(dts);
	dts = NULL;

	if (util > clc.c_m) {
		/* If utilization is above the number of cores, 
		 * it's infeasible */
		goto done;
	}
	
	if (infeas) {
		goto done;
	}

	m_low = clc.c_m - m_high;
	set_timeout(clc.c_timeout, ntasks, m_high, m_low < 0 ? 0 : m_low , util, ofile);
	if (low_sched(m_low, low)) {
		sched = 1;
	}

	if (m_low < 0) {
		m_low = 0;
	}

done:
	dts_clear(low); dts_free(low);

	fprintf(ofile, "%s\n", header());
	fprintf(ofile, "%s\n",
		summary(ntasks, infeas, sched, m_high, m_low, util));
	
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

static int
best_fit(int m_low, task_set_t** ts, task_t *task) {
	int cur = -1;
	float_t best = 1.1;
	float_t nec = task_util(task);
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
worst_fit(int m_low, task_set_t** ts, task_t *task) {
	int cur = -1;
	float_t lowest = 1.1;
	for (int i=0; i < m_low; i++) {
		float_t set_util = ts_util(ts[i]);
		if (set_util < lowest) {
			/* More capacity */
			cur = i;
			lowest = set_util;
		}
	}
	return cur;
}


static int
low_sched(int m_low, dtask_set_t* low) {
	int rv = 0;

	if (!dts_first(low) && m_low >= 0) {
		/* 0 low utilization tasks are always schedulable */
		return 1;
	}
	
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
		int idx;
		if (clc.c_best_fit) {
			idx = best_fit(m_low, parts, task);
		}
		if (clc.c_worst_fit) {
			idx = worst_fit(m_low, parts, task);
		}
		if (idx < 0) {
			goto done;
		}
		ts_add(parts[idx], task);
	}

	for (int i=0; i < m_low; i++) {
		if (clc.c_nonp) {
			/* Non preemptive */
			if (tpj(parts[i], NULL) != 0) {
				goto done;
			}
		}
		if (clc.c_p) {
			/* Preemptive */
			if (max_chunks(parts[i]) != 0) {
				goto done;
			}
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

static struct {
	int to_ntasks;
	/* not infeasible if we got this far */
	/* not schedulable if this fired */
	int to_mhigh;
	int to_mlow;
	float to_util;
	FILE *to_ofile;
} to_data;

static void
expire_timeout(int sig, siginfo_t *si, void *uc) {
	/* Clearly not safe, but that's not the point */
	fprintf(to_data.to_ofile, "%s -- TIMEOUT!\n", header());
	fprintf(to_data.to_ofile, "%s\n",
		summary(to_data.to_ntasks, 0, 0, to_data.to_mhigh, to_data.to_mlow,
			to_data.to_util));
	exit(0);
}

static void
set_timeout(int minutes, int ntasks, int mhigh, int mlow, float util, FILE *ofile) {
	timer_t timerid;
	struct sigevent sev;
	struct itimerspec its;
	sigset_t mask;
	struct sigaction sa;

	if (minutes <= 0) {
		/* No timeout */
		return;
	}

	to_data.to_ntasks = ntasks;
	to_data.to_mhigh = mhigh;
	to_data.to_mlow = mlow;
	to_data.to_util = util;
	to_data.to_ofile = ofile;
	
	minutes *= 60;
	
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = expire_timeout;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGRTMIN, &sa, NULL);

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGRTMIN;
	sev.sigev_value.sival_ptr = &timerid;
	timer_create(CLOCK_PROCESS_CPUTIME_ID, &sev, &timerid);

	memset(&its, 0, sizeof(its));
	its.it_value.tv_nsec = 0;
	its.it_value.tv_sec = minutes;

	timer_settime(timerid, 0, &its, NULL);
}

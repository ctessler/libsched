#include "taskset-config.h"

int
ts_config_process(config_t *cfg, task_set_t *ts) {
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

static void
ts_config_add_task(config_setting_t *tasks, task_t *t) {
	config_setting_t *taskg, *name, *period, *deadline, *threads, *wcet;

	taskg = config_setting_add(tasks, "", CONFIG_TYPE_GROUP);

	name = config_setting_add(taskg, "name", CONFIG_TYPE_STRING);
	config_setting_set_string(name, t->t_name);

	period = config_setting_add(taskg, "period", CONFIG_TYPE_INT);
	config_setting_set_int(period, t->t_period);

	deadline = config_setting_add(taskg, "deadline", CONFIG_TYPE_INT);
	config_setting_set_int(deadline, t->t_deadline);

	threads = config_setting_add(taskg, "threads", CONFIG_TYPE_INT);
	config_setting_set_int(threads, t->t_threads);

	wcet = config_setting_add(taskg, "wcet", CONFIG_TYPE_LIST);

	for (int i=1; i <= t->t_threads; i++) {
		config_setting_t *c;
		c = config_setting_add(wcet, NULL, CONFIG_TYPE_INT);
		config_setting_set_int(c, t->wcet(i));
	}
	
}

int
ts_config_dump(config_t *cfg, task_set_t *ts) {
	config_setting_t *root, *setting, *tasks;
	task_link_t* cookie;
	task_t* task;
	
	root = config_root_setting(cfg);

	tasks = config_setting_add(root, "tasks", CONFIG_TYPE_LIST);

	for (cookie = ts_first(ts); cookie; cookie = ts_next(ts, cookie)) {
		ts_config_add_task(tasks, ts_task(cookie));
	}
		
	return 0;
}

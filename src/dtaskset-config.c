#include "dtaskset-config.h"

int
dts_config_process(config_t *cfg, char *dir, dtask_set_t *dts) {
	char buff[1024];
	config_setting_t *setting;

	double version;
	if (!config_lookup_float(cfg, "dts-version", &version)) {
		printf("No version for the task set configuration file\n");
		return 0;
	}
	if (version != 1.0) {
		printf("Incompatible version: %.2f", version);
		return 0;
	}

	setting = config_lookup(cfg, "tasks");
	if (setting == NULL) {
		printf("No tasks in the configuration file\n");
		return 0;
	}


	int ntasks = config_setting_length(setting);
	for (int i = 0; i < ntasks; i++) {
		const char *path =
		    config_setting_get_string_elem(setting, i);

		sprintf(buff, "%s/%s", dir, path);
		dtask_t *task = dtask_read_path(buff);
		if (!task) {
			printf("Unable to read %s\n", buff);
			goto bail;
		}
		dtask_update(task);
		dtask_elem_t *e = dtse_alloc(task);
		sprintf(e->dts_path, "%s", buff);
		if (dtask_util(task) > 1) {
			e->dts_high = 1;
		}
		dts_insert_head(dts, e);
	}
	return 1;
bail:
	dts_clear(dts);
	dts_free(dts);
	return 0;
	
}

int
dts_config_dump(config_t *cfg, dtask_set_t *dts) {

}

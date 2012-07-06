#ifndef _SEMC_BATTERY_DATA_H_
#define _SEMC_BATTERY_DATA_H_

#include <linux/battery_chargalg.h>
#define SEMC_BDATA_NAME "semc_battery_data"

struct semc_battery_platform_data {
	char **supplied_to;
	size_t num_supplicants;
	u8 use_fuelgauge;
	struct device_data *ddata;

	void (*set_battery_health)(int health);
	int (*get_current_average)(void);
};

#endif /* _SEMC_BATTERY_DATA_H_ */

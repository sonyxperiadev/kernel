#ifndef _BQ27520_BATTERY_H_
#define _BQ27520_BATTERY_H_

#include <linux/power_supply.h>

#define BQ27520_NAME "bq27520"
#define BQ27520_BTBL_MAX 13

struct bq27520_block_table {
	u8 adr;
	u8 data;
};

struct bq27520_platform_data {
	const char *name;
	char **supplied_to;
	size_t num_supplicants;
	int lipo_bat_max_volt;
	int lipo_bat_min_volt;
	char *battery_dev_name;
	int polling_lower_capacity;
	int polling_upper_capacity;
	struct bq27520_block_table *udatap;
	void (*disable_algorithm)(bool);
	int (*gpio_configure)(int);
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
	u8 (*get_ac_online_status)(void);
#endif
};

int bq27520_get_current_average(void);

#endif /* BQ27520_BATTERY_H_ */

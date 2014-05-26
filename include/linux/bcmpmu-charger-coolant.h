/*
 * Generic charger coolant driver data - see
 * drivers/power/bcmpmu-charger-coolant.c
 */

#define CHARGER_CDEV_NAME "Charger_Coolant"

struct chrgr_trim_reg_data {
	u32 addr;
	u8 def_val;
};

struct charger_coolant_states {
	int state_no;
	u32 *states;
};

struct bcmpmu_cc_pdata {
	struct charger_coolant_states *coolant_states;
	struct chrgr_trim_reg_data *chrgr_trim_reg_lut;
	u32 chrgr_trim_reg_lut_sz;
	u32 coolant_poll_time;
};

int charger_cooling_get_level(struct thermal_cooling_device *cdev,
							int level_offset,
							u32 trip_current);

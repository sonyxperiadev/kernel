#ifndef _BATTERY_CHARGALG_H_
#define _BATTERY_CHARGALG_H_

#include <linux/kernel.h>
#include <linux/types.h>

#define BATTERY_CHARGALG_NAME "chargalg"

enum battery_chargalg_temperature {
	BATTERY_CHARGALG_TEMP_COLD,
	BATTERY_CHARGALG_TEMP_NORMAL,
	BATTERY_CHARGALG_TEMP_WARM,
	BATTERY_CHARGALG_TEMP_OVERHEAT,
	BATTERY_CHARGALG_TEMP_MAX_NBR,
};

enum battery_chargalg_fraction {
	BATTERY_CHARGALG_NUM,
	BATTERY_CHARGALG_DENOM,
	BATTERY_CHARGALG_FRACTION,
};

/**
 * struct battery_regulation_vs_temperature
 * @temp: The battery temperature in degree C
 * @volt: The battery voltage in mV
 * @curr: The battery current in mA
 *
 * Up to (<) temperature 'temp' charge with maximum 'volt' mV and 'curr' mA.
 */
struct battery_regulation_vs_temperature {
	s8 temp[BATTERY_CHARGALG_TEMP_MAX_NBR];
	u16 volt[BATTERY_CHARGALG_TEMP_MAX_NBR];
	u16 curr[BATTERY_CHARGALG_TEMP_MAX_NBR];
};

/**
 * struct ambient_temperature_limit
 * @base: The ambient temp limit
 * @hyst: The ambient temp hysteresis
 *
 */
struct ambient_temperature_limit {
	s8 base[BATTERY_CHARGALG_TEMP_MAX_NBR];
	s8 hyst[BATTERY_CHARGALG_TEMP_MAX_NBR];
};

#ifdef CONFIG_BATTERY_CHARGALG_ENABLE_STEP_CHARGING
struct step_charging {
	u8 c_curr[BATTERY_CHARGALG_FRACTION]; /* fraction of C */
	u16 volt;		/* mV */
	u8 volt_hysteresis_up;	/* mV */
	u8 volt_hysteresis_down;/* mV */
};
#endif

struct device_data {
	struct battery_regulation_vs_temperature *id_bat_reg;
	struct battery_regulation_vs_temperature *unid_bat_reg;
	struct ambient_temperature_limit *limit_tbl;
#ifdef CONFIG_BATTERY_CHARGALG_ENABLE_STEP_CHARGING
	struct step_charging *step_charging;
	size_t num_step_charging;
#endif
	u16 battery_capacity_mah;
	u16 maximum_charging_current_ma;
	int battery_connector_resistance;
};

struct battery_chargalg_platform_data {
	const char *name;
	char **supplied_to;
	size_t num_supplicants;

	u16 overvoltage_max_design; /* mV */
	u8 ext_eoc_recharge_enable;
	u8 recharge_threshold_capacity; /* % */
	u16 eoc_current_term; /* mA */
	s16 eoc_current_flat_time; /* seconds */
	s8 temp_hysteresis_design; /* degree C */
	u8 disable_usb_host_charging;
	struct device_data *ddata;
	char *batt_volt_psy_name;
	char *batt_curr_psy_name;

	int (*turn_on_charger)(u8 usb_compliant);
	int (*turn_off_charger)(void);
	int (*set_charger_voltage)(u16 mv);
	int (*set_charger_current)(u16 mv);
	int (*set_eoc_current_term)(u16 ma);
	int (*set_charger_safety_timer)(u16 minutes);
	int (*set_input_current_limit)(u16 ma);
	int (*set_charging_status)(int status);
	unsigned int (*get_supply_current_limit)(void);
	bool (*get_restrict_ctl)(int bvolt, u16 volt, u16 curr);
	void (*get_restricted_setting)(u16 *volt, u16 *curr);
	int (*setup_exchanged_power_supply)(u8 connection);
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
	u8 (*get_ac_online_status)(void);
	unsigned int (*get_supply_current_limit_cradle)(void);
	int (*set_input_current_limit_dual)(u16 ma_usb, u16 ma_cradle);
	int (*set_input_voltage_dpm_usb)(u8 usb_compliant);
	int (*set_input_voltage_dpm_cradle)(void);
#endif
	unsigned int allow_dynamic_charge_current_ctrl;
	u16 charge_set_current_1; /* mA */
	u16 charge_set_current_2; /* mA */
	u16 charge_set_current_3; /* mA */
	int average_current_min_limit; /* mA */
	int average_current_max_limit; /* mA */
};

extern struct device_data device_data;

void battery_chargalg_set_battery_health(int health);
void battery_chargalg_disable(bool);

#endif /* _BATTERY_CHARGALG_H_ */

#ifndef _BQ24160_CHARGER_H_
#define _BQ24160_CHARGER_H_

#define BQ24160_NAME "bq24160"

struct bq24160_platform_data {
	const char *name;
	char **supplied_to;
	size_t num_supplicants;

	u8 support_boot_charging;
	void (*notify_vbus_drop)(void);
	int (*gpio_configure)(int);
};

/**
 * bq24160_turn_on_charger() - Turns the charger HW on
 * @usb_compliant: If run in USB compliant mode or not
 *
 * The USB compliant mode will hold VBUS within the +-5 % margin.
 */
int bq24160_turn_on_charger(u8 usb_compliant);

/**
 * bq24160_turn_off_charger() - Turns the charger HW off
 *
 */
int bq24160_turn_off_charger(void);

/**
 * bq24160_set_otg_lock() - Sets the OTG locking of BQ24160
 * @lock: Setting to '1' OTG is locked.
 *
 * If driver is not yet registered in power_supply class this function will
 * return '-EAGAIN' as error code.
 */
int bq24160_set_otg_lock(int lock);

/**
 * bq24160_set_charger_voltage() - Sets the charge voltage
 * @mv: The voltage in mV.
 *
 * Setting voltage to '0' mV stops charging.
 */
int bq24160_set_charger_voltage(u16 mv);

/**
 * bq24160_set_charger_current() - Sets the charge current
 * @ma: The current in mA.
 *
 * Setting current to '0' mA stops charging.
 */
int bq24160_set_charger_current(u16 ma);

/**
 * bq24160_set_charger_termination_current() - Sets the end of charge current
 * @ma: The current in mA.
 *
 * When battery voltage has reached the charger voltage level and the charging
 * current has fallen below this current level charging is stopped,
 *
 * Setting current to '0' mA disables charge termination.
 */
int bq24160_set_charger_termination_current(u16 ma);

/**
 * bq24160_set_safety_timer() - Sets the charging safety timer
 * @minutes: The timeout in minutes.
 *
 * Setting '0' minutes disables safety timer.
 */
int bq24160_set_charger_safety_timer(u16 minutes);

/**
 * bq24160_set_input_current_limit() - Limits the current from the supplier.
 * @ma: The current in mA.
 *
 * The supplier is the i.e. USB of Wall charger.
 *
 * Setting current to '0' mA stops charging.
 */
int bq24160_set_input_current_limit(u16 ma);

/**
 * bq24160_set_input_current_limit_dual() - Limits the current from dual suppliers.
 * @ma_usb: The current from USB in mA.
 * @ma_in: The current from IN port in mA.
 *
 * The suppliers are i.e. USB and Cradle.
 *
 * Setting current to '0' mA stops charging.
 */
int bq24160_set_input_current_limit_dual(u16 ma_usb, u16 ma_in);

/**
 * bq24160_set_input_voltage_dpm_usb() - Sets input voltage based DPM(USB).
 * @usb_compliant: If run in USB compliant mode or not
 *
 */
int bq24160_set_input_voltage_dpm_usb(u8 usb_compliant);

/**
 * bq24160_set_input_voltage_dpm_in() - Sets input voltage based DPM(IN).
 *
 */
int bq24160_set_input_voltage_dpm_in(void);

/**
 * bq24160_set_external_charging_status() - Sets the status of charging
 * @status: The POWER_SUPPLY_STATUS_* or -1 to release external value
 *
 */
int bq24160_set_ext_charging_status(int status);

/**
 * bq24160_charger_initialized() - Gets whether chrger was initilized or not.
 * return 0 as charger was not initilized
 *
 */
int bq24160_charger_initialized(void);

/**
 * bq24160_is_restricted_by_charger_revision() - Gets restricted control.
 * @bvolt: battery voltage
 * @volt: charge voltage setting now
 * @curr: charge current setting now
 * return true:restricted false:Not restricted
 *
 */
bool bq24160_is_restricted_by_charger_revision(int bvolt, u16 volt, u16 curr);

/**
 * bq24160_get_restricted_setting() - Gets restricted voltage and current.
 * @volt: restricted charge voltage that specified from charger
 * @curr: restricted charge current that specified from charger
 *
 */
void bq24160_get_restricted_setting(u16 *volt, u16 *curr);

/**
 * bq24160_setup_exchanged_power_supply() - Setup when power supply connection
 *                                          is exchanged.
 * @connection: Connection status of charger.
 *              0x01: USB charger is connected.
 *              0x02: Wall charger is connected.
 *              0x04: Cradle charger is connected.
 *
 */
int bq24160_setup_exchanged_power_supply(u8 connection);

#endif

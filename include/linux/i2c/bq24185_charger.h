#ifndef _BQ24185_CHARGER_H_
#define _BQ24185_CHARGER_H_

#define BQ24185_NAME "bq24185"

/* Reference sense resistor (mOhm) */
#define BQ24185_RSENS_REF 68

/* Maximum battery regulation voltage */
#define BQ24185_MBRV_MV_4200 0x00
#define BQ24185_MBRV_MV_20   (1<<0)
#define BQ24185_MBRV_MV_40   (1<<1)
#define BQ24185_MBRV_MV_80   (1<<2)
#define BQ24185_MBRV_MV_160  (1<<3)

/* Maximum charge current sense voltage */
#define BQ24185_MCCSV_MV_37p4   0x00
#define BQ24185_MCCSV_MV_6p8    (1<<4)
#define BQ24185_MCCSV_MV_13p6   (1<<5)
#define BQ24185_MCCSV_MV_27p2   (1<<6)
#define BQ24185_MCCSV_MV_54p4   (1<<7)

enum bq24185_vindpm {
	VINDPM_4150MV,
	VINDPM_4230MV,
	VINDPM_4310MV,
	VINDPM_4390MV,
	VINDPM_4470MV,
	VINDPM_4550MV,
	VINDPM_4630MV,
	VINDPM_4710MV,
};

enum bq24185_opa_mode {
	CHARGER_CHARGER_MODE,
	CHARGER_BOOST_MODE
};

struct bq24185_platform_data {
	const char *name;
	char **supplied_to;
	size_t num_supplicants;

	u8 support_boot_charging;
	u8 rsens;
	int mbrv;	/* Max battery regulation voltage (BQ24185_MBRV_*)   */
	int mccsv;	/* Max charge current sense voltage(BQ24185_MCCSV_*) */
	void (*notify_vbus_drop)(void);
	int (*gpio_configure)(int);
	u8 vindpm_usb_compliant;
	u8 vindpm_non_compliant;
};

/**
 * bq24185_turn_on_charger() - Turns the charger HW on
 * @usb_compliant: If run in USB compliant mode or not
 *
 * The USB compliant mode will hold VBUS within the +-5 % margin.
 */
int bq24185_turn_on_charger(u8 usb_compliant);

/**
 * bq24185_turn_off_charger() - Turns the charger HW off
 *
 */
int bq24185_turn_off_charger(void);

/**
 * bq24185_set_opa_mode() - Sets the operating mode of BQ24185
 * @mode: The operating mode
 *
 * If driver is not yet registered in power_supply class this function will
 * return '-EAGAIN' as error code.
 */
int bq24185_set_opa_mode(enum bq24185_opa_mode mode);

/**
 * bq24185_get_opa_mode() - Gets the operating mode of BQ24185
 * @mode : current opa_mode
 *
 * If driver is not yet registered in power_supply class this function will
 * return '-EAGAIN' as error code.
 *
 */
int bq24185_get_opa_mode(enum bq24185_opa_mode *mode);

/**
 * bq24185_set_charger_voltage() - Sets the charge voltage
 * @mv: The voltage in mV.
 *
 * Setting voltage to '0' mV stops charging.
 */
int bq24185_set_charger_voltage(u16 mv);

/**
 * bq24185_set_charger_current() - Sets the charge current
 * @ma: The current in mA.
 *
 * Setting current to '0' mA stops charging.
 */
int bq24185_set_charger_current(u16 ma);

/**
 * bq24185_set_charger_termination_current() - Sets the end of charge current
 * @ma: The current in mA.
 *
 * When battery voltage has reached the charger voltage level and the charging
 * current has fallen below this current level charging is stopped,
 *
 * Setting current to '0' mA disables charge termination.
 */
int bq24185_set_charger_termination_current(u16 ma);

/**
 * bq24185_set_safety_timer() - Sets the charging safety timer
 * @minutes: The timeout in minutes.
 *
 * Setting '0' minutes disables safety timer.
 */
int bq24185_set_charger_safety_timer(u16 minutes);

/**
 * bq24185_set_input_current_limit() - Limits the current from the supplier.
 * @ma: The current in mA.
 *
 * The supplier is the i.e. USB of Wall charger.
 *
 * Setting current to '0' mA stops charging.
 */
int bq24185_set_input_current_limit(u16 ma);

/**
 * bq24185_set_external_charging_status() - Sets the status of charging
 * @status: The POWER_SUPPLY_STATUS_* or -1 to release external value
 *
 */
int bq24185_set_ext_charging_status(int status);

/**
 * bq24185_charger_initialized() - Gets whether chrger was initilized or not.
 * return 0 as charger was not initilized
 *
 */
int bq24185_charger_initialized(void);

#endif

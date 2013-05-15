/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/leds.h>
#include <linux/leds-pm8xxx.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#include <linux/gpio_keys.h>
#include <asm/mach-types.h>
#include <asm/mach/mmc.h>
#include <mach/msm_bus_board.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/restart.h>
#include "devices.h"
#include "board-8064.h"
#include "board-sony_fusion3.h"
#include "charger-sony_fusion3.h"

struct pm8xxx_gpio_init {
	unsigned			gpio;
	struct pm_gpio			config;
};

struct pm8xxx_mpp_init {
	unsigned			mpp;
	struct pm8xxx_mpp_config_data	config;
};

#define PM8921_GPIO_INIT(_gpio, _dir, _buf, _val, _pull, _vin, _out_strength, \
			_func, _inv, _disable) \
{ \
	.gpio	= PM8921_GPIO_PM_TO_SYS(_gpio), \
	.config	= { \
		.direction	= _dir, \
		.output_buffer	= _buf, \
		.output_value	= _val, \
		.pull		= _pull, \
		.vin_sel	= _vin, \
		.out_strength	= _out_strength, \
		.function	= _func, \
		.inv_int_pol	= _inv, \
		.disable_pin	= _disable, \
	} \
}

#define PM8921_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8921_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8921_MPP_DISABLE(_mpp) \
	PM8921_MPP_INIT(_mpp, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE)

#define PM8821_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8821_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8921_GPIO_DISABLE(_gpio) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, 0, 0, 0, PM_GPIO_VIN_S4, \
			 0, 0, 0, 1)

#define PM8921_GPIO_DISABLE_HIZ(_gpio) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, PM_GPIO_OUT_BUF_CMOS, 0, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_NO, \
			PM_GPIO_FUNC_NORMAL, 0, 1)

#define PM8921_GPIO_OUTPUT(_gpio, _val, _strength) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_##_strength, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_OUTPUT_BUFCONF(_gpio, _val, _strength, _bufconf) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT,\
			PM_GPIO_OUT_BUF_##_bufconf, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_##_strength, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_INPUT(_gpio, _pull) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, PM_GPIO_OUT_BUF_CMOS, 0, \
			_pull, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_NO, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_OUTPUT_FUNC(_gpio, _val, _func) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_HIGH, \
			_func, 0, 0)

#define PM8921_GPIO_OUTPUT_VIN(_gpio, _val, _vin) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, _vin, \
			PM_GPIO_STRENGTH_HIGH, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_OUTPUT_BUFCONF_VPH(_gpio, _val, _strength, _bufconf) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT,\
			PM_GPIO_OUT_BUF_##_bufconf, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_VPH, \
			PM_GPIO_STRENGTH_##_strength, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

/* Initial PM8921 GPIO configurations */
static struct pm8xxx_gpio_init pm8921_gpios[] __initdata = {
	PM8921_GPIO_DISABLE_HIZ(1), /* Not connected */
	PM8921_GPIO_DISABLE_HIZ(2), /* Not connected */
	/* GPIO_3 (KYPD_SNS3) set in pm8921_odin_*_gpios */
	/* GPIO_4 (KYPD_SNS4) set in pm8921_odin_*_gpios */
	PM8921_GPIO_OUTPUT(5, 0, LOW), /* IR_LEVEL_EN */
	PM8921_GPIO_DISABLE_HIZ(6), /* HW_ID[0], used only by S1boot*/
	PM8921_GPIO_DISABLE_HIZ(7), /* HW_ID[1], used only by S1boot*/
	PM8921_GPIO_DISABLE_HIZ(8), /* HW_ID[2], used only by S1boot*/
	PM8921_GPIO_DISABLE_HIZ(9), /* HW_ID[3], used only by S1boot*/
	PM8921_GPIO_DISABLE_HIZ(10), /* Not connected */
	PM8921_GPIO_DISABLE_HIZ(11), /* Not connected */
	/* GPIO_12 set in pm8921_odin_*_gpios */
	PM8921_GPIO_OUTPUT(13, 0, LOW), /* vib reset          */
	PM8921_GPIO_OUTPUT(14, 1, LOW), /* IR reset          */
	PM8921_GPIO_DISABLE_HIZ(15), /* Not connected */
	PM8921_GPIO_DISABLE_HIZ(16), /* Not connected */
	PM8921_GPIO_DISABLE_HIZ(17), /* Not connected */
	PM8921_GPIO_DISABLE_HIZ(18),        /* WLAN/BT self test  */
	PM8921_GPIO_OUTPUT(19, 0, MED), /* Right speaker enab */
	PM8921_GPIO_INPUT(20, PM_GPIO_PULL_NO), /* USB over curr det */
	PM8921_GPIO_OUTPUT(21, 0, LOW), /* NFC dwld enable    */
	PM8921_GPIO_OUTPUT(22, 0, HIGH), /* RF ID enable     */
	PM8921_GPIO_INPUT(23, PM_GPIO_PULL_NO), /* LCD ID */
	PM8921_GPIO_OUTPUT(24, 0, LOW), /* LCD_DCDC_EN */
	PM8921_GPIO_OUTPUT(25, 0, LOW), /* DISP_RESET_N */
	PM8921_GPIO_OUTPUT(26, 1, LOW), /* lm3533 hwen pin    */
	PM8921_GPIO_OUTPUT(27, 0, LOW), /* MHL reset          */
	PM8921_GPIO_OUTPUT(28, 0, LOW), /* Main camera reset  */
	/* GPIO_29 set in pm8921_odin_*_gpios */
	PM8921_GPIO_DISABLE_HIZ(30), /* Not connected */
	PM8921_GPIO_DISABLE_HIZ(31), /* Not connected */
	PM8921_GPIO_DISABLE_HIZ(32), /* Not connected */
	/* NFC enable */
	PM8921_GPIO_OUTPUT_BUFCONF_VPH(33, 0, LOW, OPEN_DRAIN),
	/* TABLA CODEC RESET */
	PM8921_GPIO_OUTPUT(34, 1, HIGH),
	PM8921_GPIO_DISABLE_HIZ(35), /* Not connected */
	PM8921_GPIO_DISABLE_HIZ(36), /* Not connected */
	PM8921_GPIO_DISABLE_HIZ(37), /* Not connected */
	/* GPIO_38 (VOLUME_DOWN_KEY) set in pm8921_odin_*_gpios */
	PM8921_GPIO_INPUT(39, PM_GPIO_PULL_NO), /* SSBI_PMIC_FWD_CLK */
	PM8921_GPIO_DISABLE_HIZ(40), /* Not connected */
	PM8921_GPIO_DISABLE_HIZ(41), /* Not connected */
	PM8921_GPIO_OUTPUT_BUFCONF_VPH(42, 1, LOW, CMOS), /* OTG_OVP_CNTL */
	PM8921_GPIO_DISABLE_HIZ(43), /* Not connected */
	/* GPIO_44 set in pm8921_odin_*_gpios */
};

static struct pm8xxx_gpio_init pm8921_odin_sp1_gpios[] __initdata = {
	PM8921_GPIO_INPUT(3, PM_GPIO_PULL_UP_30),  /* Camera focus */
	PM8921_GPIO_INPUT(4, PM_GPIO_PULL_UP_30),  /* Camera snapshot */
	PM8921_GPIO_OUTPUT(12, 0, LOW),            /* vib enable */
	PM8921_GPIO_DISABLE_HIZ(29),               /* Not connected */
	PM8921_GPIO_INPUT(38, PM_GPIO_PULL_UP_30), /* Volume Down */
	PM8921_GPIO_INPUT(44, PM_GPIO_PULL_UP_30), /* Volume Up */
};

static struct pm8xxx_gpio_init pm8921_odin_sp1_pb_gpios[] __initdata = {
	PM8921_GPIO_INPUT(3, PM_GPIO_PULL_UP_30),  /* Camera focus */
	PM8921_GPIO_INPUT(4, PM_GPIO_PULL_UP_30),  /* Camera snapshot */
	PM8921_GPIO_OUTPUT(12, 0, LOW),            /* vib enable */
	PM8921_GPIO_INPUT(29, PM_GPIO_PULL_UP_30), /* Volume Up */
	PM8921_GPIO_INPUT(38, PM_GPIO_PULL_UP_30), /* Volume Down */
	PM8921_GPIO_DISABLE_HIZ(44),               /* Not connected */
};

static struct pm8xxx_gpio_init pm8921_odin_sp11_gpios[] __initdata = {
	PM8921_GPIO_INPUT(3, PM_GPIO_PULL_UP_30),  /* Camera focus */
	PM8921_GPIO_INPUT(4, PM_GPIO_PULL_UP_30),  /* Camera snapshot */
	PM8921_GPIO_DISABLE_HIZ(12),               /* Not connected */
	PM8921_GPIO_INPUT(29, PM_GPIO_PULL_UP_30), /* Volume Up */
	PM8921_GPIO_INPUT(38, PM_GPIO_PULL_UP_30), /* Volume Down */
	PM8921_GPIO_DISABLE_HIZ(44),               /* Not connected */
};

static struct pm8xxx_gpio_init pm8921_cdp_kp_gpios[] __initdata = {
	PM8921_GPIO_INPUT(37, PM_GPIO_PULL_UP_30),
};

/* Initial PM8XXX MPP configurations */
static struct pm8xxx_mpp_init pm8xxx_mpps[] __initdata = {
	PM8921_MPP_DISABLE(1), /* N/C */
	PM8921_MPP_DISABLE(2), /* N/C */
	PM8921_MPP_DISABLE(3), /* N/C, Reserved for PHF_ADC */
	PM8921_MPP_DISABLE(4), /* N/C */
	/* MPP 5 and 6 are handled by QCT*/
	/* External 5V regulator enable; used by USB_OTG switches. */
	PM8921_MPP_INIT(7, D_OUTPUT, PM8921_MPP_DIG_LEVEL_S4, DOUT_CTRL_LOW),
	/* BL_THERM */
	PM8921_MPP_INIT(8, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH5, DOUT_CTRL_LOW),
	/* RF_ID */
	PM8921_MPP_INIT(9, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH6, DOUT_CTRL_LOW),
	PM8921_MPP_DISABLE(10), /* N/C */
	PM8921_MPP_DISABLE(11), /* N/C, Reserve Audio Study */
	PM8921_MPP_DISABLE(12), /* N/C */
};

void __init apq8064_pm8xxx_gpio_mpp_init(void)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(pm8921_gpios); i++) {
		rc = pm8xxx_gpio_config(pm8921_gpios[i].gpio,
					&pm8921_gpios[i].config);
		if (rc) {
			pr_err("%s: pm8xxx_gpio_config: rc=%d\n", __func__, rc);
			break;
		}
	}

	if (machine_is_apq8064_cdp() || machine_is_apq8064_liquid())
		for (i = 0; i < ARRAY_SIZE(pm8921_cdp_kp_gpios); i++) {
			rc = pm8xxx_gpio_config(pm8921_cdp_kp_gpios[i].gpio,
						&pm8921_cdp_kp_gpios[i].config);
			if (rc) {
				pr_err("%s: pm8xxx_gpio_config: rc=%d\n",
					__func__, rc);
				break;
			}
		}

	if (sony_hw_rev() <= HW_REV_ODIN_SP1) {
		for (i = 0; i < ARRAY_SIZE(pm8921_odin_sp1_gpios); i++) {
			rc = pm8xxx_gpio_config(pm8921_odin_sp1_gpios[i].gpio,
						&pm8921_odin_sp1_gpios[i].config);
			if (rc) {
				pr_err("%s: pm8xxx_gpio_config: rc=%d\n",
					__func__, rc);
				break;
			}
		}
	} else if (sony_hw_rev() == HW_REV_ODIN_SP1_PB) {
		for (i = 0; i < ARRAY_SIZE(pm8921_odin_sp1_pb_gpios); i++) {
			rc = pm8xxx_gpio_config(
					pm8921_odin_sp1_pb_gpios[i].gpio,
					&pm8921_odin_sp1_pb_gpios[i].config);
			if (rc) {
				pr_err("%s: pm8xxx_gpio_config: rc=%d\n",
					__func__, rc);
				break;
			}
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(pm8921_odin_sp11_gpios); i++) {
			rc = pm8xxx_gpio_config(pm8921_odin_sp11_gpios[i].gpio,
						&pm8921_odin_sp11_gpios[i].config);
			if (rc) {
				pr_err("%s: pm8xxx_gpio_config: rc=%d\n",
					__func__, rc);
				break;
			}
		}
	}

	for (i = 0; i < ARRAY_SIZE(pm8xxx_mpps); i++) {
		rc = pm8xxx_mpp_config(pm8xxx_mpps[i].mpp,
					&pm8xxx_mpps[i].config);
		if (rc) {
			pr_err("%s: pm8xxx_mpp_config: rc=%d\n", __func__, rc);
			break;
		}
	}
}

static struct pm8xxx_pwrkey_platform_data apq8064_pm8921_pwrkey_pdata = {
	.pull_up		= 1,
	.kpd_trigger_delay_us	= 15625,
	.wakeup			= 1,
};

static struct pm8xxx_misc_platform_data apq8064_pm8921_misc_pdata = {
	.priority		= 0,
};

#define PM8921_LC_LED_MAX_CURRENT	4	/* I = 4mA */
#define PM8921_LC_LED_LOW_CURRENT	1	/* I = 1mA */
#define PM8XXX_LED_PWM_PERIOD		1000
#define PM8XXX_LED_PWM_DUTY_MS		20
/**
 * PM8XXX_PWM_CHANNEL_NONE shall be used when LED shall not be
 * driven using PWM feature.
 */
#define PM8XXX_PWM_CHANNEL_NONE		-1

static struct led_info pm8921_led_info[] = {
	[0] = {
		.name			= "led:red",
		.default_trigger	= "ac-online",
	},
};

static struct led_platform_data pm8921_led_core_pdata = {
	.num_leds = ARRAY_SIZE(pm8921_led_info),
	.leds = pm8921_led_info,
};

static int pm8921_led0_pwm_duty_pcts[56] = {
	1, 4, 8, 12, 16, 20, 24, 28, 32, 36,
	40, 44, 46, 52, 56, 60, 64, 68, 72, 76,
	80, 84, 88, 92, 96, 100, 100, 100, 98, 95,
	92, 88, 84, 82, 78, 74, 70, 66, 62, 58,
	58, 54, 50, 48, 42, 38, 34, 30, 26, 22,
	14, 10, 6, 4, 1
};

/*
 * Note: There is a bug in LPG module that results in incorrect
 * behavior of pattern when LUT index 0 is used. So effectively
 * there are 63 usable LUT entries.
 */
static struct pm8xxx_pwm_duty_cycles pm8921_led0_pwm_duty_cycles = {
	.duty_pcts = (int *)&pm8921_led0_pwm_duty_pcts,
	.num_duty_pcts = ARRAY_SIZE(pm8921_led0_pwm_duty_pcts),
	.duty_ms = PM8XXX_LED_PWM_DUTY_MS,
	.start_idx = 1,
};

static struct pm8xxx_led_config pm8921_led_configs[] = {
	[0] = {
		.id = PM8XXX_ID_LED_0,
		.mode = PM8XXX_LED_MODE_PWM2,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
		.pwm_channel = 5,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
	},
};

static struct pm8xxx_led_platform_data apq8064_pm8921_leds_pdata = {
		.led_core = &pm8921_led_core_pdata,
		.configs = pm8921_led_configs,
		.num_configs = ARRAY_SIZE(pm8921_led_configs),
};

static struct pm8xxx_adc_amux apq8064_pm8921_adc_channels_data[] = {
	{"vcoin", CHANNEL_VCOIN, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vbat", CHANNEL_VBAT, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"dcin", CHANNEL_DCIN, CHAN_PATH_SCALING4, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ichg", CHANNEL_ICHG, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vph_pwr", CHANNEL_VPH_PWR, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ibat", CHANNEL_IBAT, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"batt_therm", CHANNEL_BATT_THERM, CHAN_PATH_SCALING1, AMUX_RSV2,
		ADC_DECIMATION_TYPE2, ADC_SCALE_BATT_THERM},
	{"batt_id", CHANNEL_BATT_ID, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"usbin", CHANNEL_USBIN, CHAN_PATH_SCALING3, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pmic_therm", CHANNEL_DIE_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PMIC_THERM},
	{"625mv", CHANNEL_625MV, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"125v", CHANNEL_125V, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"chg_temp", CHANNEL_CHG_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pba_therm", CHANNEL_MUXOFF, CHAN_PATH_SCALING1, AMUX_RSV0,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PBA_THERM},
	{"bl_therm", ADC_MPP_1_AMUX5, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_BL_THERM},
	{"rf_id", ADC_MPP_1_AMUX6, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"apq_therm", ADC_MPP_1_AMUX3, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PA_THERM},
};

static struct pm8xxx_adc_properties apq8064_pm8921_adc_data = {
	.adc_vdd_reference	= 1800, /* milli-voltage for this adc */
	.bitresolution		= 15,
	.bipolar                = 0,
};

static struct pm8xxx_adc_platform_data apq8064_pm8921_adc_pdata = {
	.adc_channel		= apq8064_pm8921_adc_channels_data,
	.adc_num_board_channel	= ARRAY_SIZE(apq8064_pm8921_adc_channels_data),
	.adc_prop		= &apq8064_pm8921_adc_data,
	.adc_mpp_base		= PM8921_MPP_PM_TO_SYS(1),
};

static struct pm8xxx_mpp_platform_data
apq8064_pm8921_mpp_pdata __devinitdata = {
	.mpp_base	= PM8921_MPP_PM_TO_SYS(1),
};

static struct pm8xxx_gpio_platform_data
apq8064_pm8921_gpio_pdata __devinitdata = {
	.gpio_base	= PM8921_GPIO_PM_TO_SYS(1),
};

static struct pm8xxx_irq_platform_data
apq8064_pm8921_irq_pdata __devinitdata = {
	.irq_base		= PM8921_IRQ_BASE,
	.devirq			= MSM_GPIO_TO_INT(74),
	.irq_trigger_flag	= IRQF_TRIGGER_LOW,
	.dev_id			= 0,
};

static struct pm8xxx_rtc_platform_data
apq8064_pm8921_rtc_pdata = {
#ifdef CONFIG_RTC_SEMC_ETS
	.rtc_write_enable       = true,
#else
	.rtc_write_enable       = false,
#endif
	.rtc_alarm_powerup      = false,
};

static int apq8064_pm8921_therm_mitigation[] = {
	1525,
	825,
	475,
	325,
};

#define MAX_VOLTAGE_MV	4200
#define V_CUTOFF_MV	3200
#define CHG_TERM_MA	115
static struct pm8921_charger_platform_data
apq8064_pm8921_chg_pdata __devinitdata = {
	.ttrkl_time		= 64,
	.update_time		= 30000,
#ifdef CONFIG_PM8921_SONY_BMS_CHARGER
	.update_time_at_low_bat = 1000,
	.alarm_low_mv		= V_CUTOFF_MV,
	.alarm_high_mv		= V_CUTOFF_MV + 100,
#endif
	.max_voltage		= MAX_VOLTAGE_MV,
	.min_voltage		= V_CUTOFF_MV,
	.resume_voltage_delta   = 120,
	.resume_charge_percent  = 95,
	.term_current		= CHG_TERM_MA,
	.cool_temp		= 10,
	.warm_temp		= 45,
	.hysteresis_temp	= 3,
	.temp_check_period	= 1,
	.safe_current_ma	= 1525,
	.max_bat_chg_current	= 1525,
	.cool_bat_chg_current	= 1525,
	.warm_bat_chg_current	= 325,
	.cool_bat_voltage	= 4200,
	.warm_bat_voltage	= 4000,
	.ibat_calib_enable	= 1,
	.thermal_mitigation	= apq8064_pm8921_therm_mitigation,
	.thermal_levels		= ARRAY_SIZE(apq8064_pm8921_therm_mitigation),
	.rconn_mohm		= 18,
	.btc_override		= 1,
	.btc_override_cold_degc	= 5,
	.btc_override_hot_degc	= 55,
	.btc_delay_ms		= 10000,
	.btc_panic_if_cant_stop_chg	= 1,
	.stop_chg_upon_expiry   = 1,
	.safety_time		= 512,
	.soc_scaling		= 1,
};

static struct pm8xxx_ccadc_platform_data
apq8064_pm8xxx_ccadc_pdata = {
	.r_sense_uohm		= 10000,
	.calib_delay_ms		= 600000,
};

static struct pm8921_bms_platform_data
apq8064_pm8921_bms_pdata __devinitdata = {
#ifdef CONFIG_PM8921_SONY_BMS_CHARGER
	.battery_data			= &pm8921_battery_data,
	.allow_soc_increase		= true,
#else
	.battery_type			= BATT_OEM,
#endif
	.r_sense_uohm			= 10000,
	.v_cutoff			= V_CUTOFF_MV,
	.i_test				= 1000,
	.max_voltage_uv			= MAX_VOLTAGE_MV * 1000,
	.rconn_mohm			= 20,
#ifndef CONFIG_PM8921_SONY_BMS_CHARGER
	.alarm_low_mv			= V_CUTOFF_MV,
	.alarm_high_mv			= V_CUTOFF_MV + 100,
#endif
	.shutdown_soc_valid_limit	= 20,
	.adjust_soc_low_threshold	= 25,
	.chg_term_ua			= CHG_TERM_MA * 1000,
	.enable_fcc_learning		= 1,
	.normal_voltage_calc_ms		= 20000,
	.low_voltage_calc_ms		= 1000,
#ifndef CONFIG_PM8921_SONY_BMS_CHARGER
	.low_voltage_detect		= 1,
	.vbatt_cutoff_retries		= 5,
	.high_ocv_correction_limit_uv	= 50,
	.low_ocv_correction_limit_uv	= 100,
	.hold_soc_est			= 3,
#endif
};

static struct pm8921_platform_data
apq8064_pm8921_platform_data __devinitdata = {
	.regulator_pdatas	= msm8064_pm8921_regulator_pdata,
	.irq_pdata		= &apq8064_pm8921_irq_pdata,
	.gpio_pdata		= &apq8064_pm8921_gpio_pdata,
	.mpp_pdata		= &apq8064_pm8921_mpp_pdata,
	.rtc_pdata		= &apq8064_pm8921_rtc_pdata,
	.pwrkey_pdata		= &apq8064_pm8921_pwrkey_pdata,
	.misc_pdata		= &apq8064_pm8921_misc_pdata,
	.leds_pdata		= &apq8064_pm8921_leds_pdata,
	.adc_pdata		= &apq8064_pm8921_adc_pdata,
	.charger_pdata		= &apq8064_pm8921_chg_pdata,
	.bms_pdata		= &apq8064_pm8921_bms_pdata,
	.ccadc_pdata		= &apq8064_pm8xxx_ccadc_pdata,
};

static struct pm8xxx_irq_platform_data
apq8064_pm8821_irq_pdata __devinitdata = {
	.irq_base		= PM8821_IRQ_BASE,
	.devirq			= PM8821_SEC_IRQ_N,
	.irq_trigger_flag	= IRQF_TRIGGER_HIGH,
	.dev_id			= 1,
};

static struct pm8xxx_mpp_platform_data
apq8064_pm8821_mpp_pdata __devinitdata = {
	.mpp_base	= PM8821_MPP_PM_TO_SYS(1),
};

static struct pm8821_platform_data
apq8064_pm8821_platform_data __devinitdata = {
	.irq_pdata	= &apq8064_pm8821_irq_pdata,
	.mpp_pdata	= &apq8064_pm8821_mpp_pdata,
};

static struct msm_ssbi_platform_data apq8064_ssbi_pm8921_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name		= "pm8921-core",
		.platform_data	= &apq8064_pm8921_platform_data,
	},
};

static struct msm_ssbi_platform_data apq8064_ssbi_pm8821_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name		= "pm8821-core",
		.platform_data	= &apq8064_pm8821_platform_data,
	},
};

void __init apq8064_init_pmic(void)
{
	pmic_reset_irq = PM8921_IRQ_BASE + PM8921_RESOUT_IRQ;

	apq8064_device_ssbi_pmic1.dev.platform_data =
						&apq8064_ssbi_pm8921_pdata;
	apq8064_device_ssbi_pmic2.dev.platform_data =
				&apq8064_ssbi_pm8821_pdata;
	apq8064_pm8921_platform_data.num_regulators =
					msm8064_pm8921_regulator_pdata_len;

	if (sony_hw_rev() < HW_REV_ODIN_AP1) {
		int i;
		/* Need to set active pullups on usb_otg and hdmi_mvs for
		 * pre-AP1 revisions to avoid current leakage */
		for (i = 0; i < msm8064_pm8921_regulator_pdata_len; i++) {
			if (!strcmp(msm8064_pm8921_regulator_pdata[i].init_data.
				constraints.name, "8921_usb_otg") ||
				!strcmp(msm8064_pm8921_regulator_pdata[i].
				init_data.constraints.name, "8921_hdmi_mvs")){

				printk(KERN_INFO"%s: Set active pullup on %s\n",
					__func__,
					msm8064_pm8921_regulator_pdata[i].
					init_data.constraints.name);

				msm8064_pm8921_regulator_pdata[i].
					pull_down_enable = 1;
			}
		}
	}

	if (machine_is_apq8064_mtp()) {
		apq8064_pm8921_bms_pdata.battery_type = BATT_PALLADIUM;
	} else if (machine_is_apq8064_liquid()) {
		apq8064_pm8921_bms_pdata.battery_type = BATT_DESAY;
	}
}

static struct gpio_keys_button odin_sp1_keys[] = {
	{
		.code           = KEY_CAMERA_FOCUS,
		.gpio           = PM8921_GPIO_PM_TO_SYS(3),
		.desc           = "cam_focus_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = PM8921_GPIO_PM_TO_SYS(44),
		.desc           = "volume_up_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = PM8921_GPIO_PM_TO_SYS(38),
		.desc           = "volume_down_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_CAMERA_SNAPSHOT,
		.gpio           = PM8921_GPIO_PM_TO_SYS(4),
		.desc           = "cam_snap_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
};

static struct gpio_keys_button odin_sp11_keys[] = {
	{
		.code           = KEY_CAMERA_FOCUS,
		.gpio           = PM8921_GPIO_PM_TO_SYS(3),
		.desc           = "cam_focus_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = PM8921_GPIO_PM_TO_SYS(29),
		.desc           = "volume_up_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = PM8921_GPIO_PM_TO_SYS(38),
		.desc           = "volume_down_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_CAMERA_SNAPSHOT,
		.gpio           = PM8921_GPIO_PM_TO_SYS(4),
		.desc           = "cam_snap_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
};

static struct gpio_keys_platform_data odin_keys_data = {
	.buttons        = odin_sp11_keys,
	.nbuttons       = ARRAY_SIZE(odin_sp11_keys),
};

static struct platform_device odin_kp_pdev = {
	.name           = "gpio-keys",
	.id             = -1,
	.dev            = {
		.platform_data  = &odin_keys_data,
	},
};

static int __init input_devices_init(void)
{
	if (sony_hw_rev() <= HW_REV_ODIN_SP1) {
		odin_keys_data.buttons = odin_sp1_keys;
		odin_keys_data.nbuttons = ARRAY_SIZE(odin_sp1_keys);
	}
	platform_device_register(&odin_kp_pdev);
	return 0;
}

static void __exit input_devices_exit(void)
{
}

module_init(input_devices_init);
module_exit(input_devices_exit);

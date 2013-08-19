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
#include <linux/mfd/pm8xxx/vibrator.h>
#include <linux/gpio_event.h>
#include <linux/gpio_keys.h>
#include <asm/mach-types.h>
#include <asm/mach/mmc.h>
#include <mach/msm_bus_board.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/restart.h>
#include "devices.h"
#include "board-8064.h"
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

#define PM8821_MPP_DISABLE(_mpp) \
	PM8821_MPP_INIT(_mpp, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE)

#define PM8921_GPIO_DISABLE(_gpio) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, 0, 0, 0, PM_GPIO_VIN_S4, \
			 0, 0, 0, 1)

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
	PM8921_GPIO_DISABLE(1),                    /* NC */
	PM8921_GPIO_DISABLE(2),                    /* NC */
	PM8921_GPIO_INPUT(3, PM_GPIO_PULL_UP_30),  /* CAMERA AF Key */
	PM8921_GPIO_INPUT(4, PM_GPIO_PULL_UP_30),  /* CAMERA Shutter Key */
	PM8921_GPIO_DISABLE(5),                    /* NC */
	PM8921_GPIO_INPUT(6,  PM_GPIO_PULL_NO),    /* HW_ID[0] */
	PM8921_GPIO_INPUT(7,  PM_GPIO_PULL_NO),    /* HW_ID[1] */
	PM8921_GPIO_INPUT(8,  PM_GPIO_PULL_NO),    /* HW_ID[2] */
	PM8921_GPIO_DISABLE(9),                    /* NC */
	PM8921_GPIO_DISABLE(10),                   /* NC */
	PM8921_GPIO_DISABLE(11),                   /* NC */
	PM8921_GPIO_DISABLE(12),                   /* NC */
	PM8921_GPIO_DISABLE(13),                   /* NC */
	PM8921_GPIO_DISABLE(14),                   /* NC */
	PM8921_GPIO_DISABLE(15),                   /* NC */
	PM8921_GPIO_DISABLE(16),                   /* NC */
	PM8921_GPIO_DISABLE(17),                   /* NC */
	PM8921_GPIO_DISABLE(18),                   /* NC */
	PM8921_GPIO_OUTPUT(19, 0, MED), /* Right speaker enab */
	PM8921_GPIO_INPUT(20, PM_GPIO_PULL_NO),    /* OTG_OVRCUR_DET_N */
	PM8921_GPIO_OUTPUT(21, 0, LOW),            /* NFC_DWLD_EN */
	PM8921_GPIO_OUTPUT(22, 0, HIGH),           /* RF_ID_EN */
	PM8921_GPIO_DISABLE(23),                   /* NC */
	PM8921_GPIO_OUTPUT(24, 0, LOW), /* LCD_DCDC_EN */
	PM8921_GPIO_OUTPUT(25, 0, LOW), /* DISP_RESET_N */
	PM8921_GPIO_OUTPUT(26, 0, LOW),            /* LMU_EN */
	PM8921_GPIO_OUTPUT(27, 0, LOW),            /* MHL_RST_N */
	PM8921_GPIO_OUTPUT(28, 0, LOW),            /* MCAM_RST_N */
	PM8921_GPIO_INPUT(29, PM_GPIO_PULL_UP_30), /* VOLUME_UP_KEY */
	PM8921_GPIO_DISABLE(30),                   /* NC */
	PM8921_GPIO_DISABLE(31),                   /* NC */
	PM8921_GPIO_DISABLE(32),                   /* NC */
	PM8921_GPIO_OUTPUT_BUFCONF_VPH(33, 0, LOW, OPEN_DRAIN), /* NFC_EXT_EN */
	PM8921_GPIO_OUTPUT(34, 1, HIGH),           /* WCD9310_RESET_N */
	PM8921_GPIO_DISABLE(35),                   /* NC */
	PM8921_GPIO_DISABLE(36),                   /* NC */
	PM8921_GPIO_DISABLE(37),                   /* NC */
	PM8921_GPIO_INPUT(38, PM_GPIO_PULL_UP_30),/* VOLUME_DOWN_KEY */
	/* GPIO_39 (SSBI_PMIC_FWD_CLK) is set by PBL */
	PM8921_GPIO_DISABLE(40),                   /* NC */
	PM8921_GPIO_DISABLE(41),                   /* NC */
	PM8921_GPIO_OUTPUT_BUFCONF_VPH(42, 1, LOW, CMOS), /* OTG_OVP_CNTL */
	PM8921_GPIO_DISABLE(43),                    /* NC */
	PM8921_GPIO_DISABLE(44),                    /* NC */
};

static struct pm8xxx_gpio_init pm8921_mtp_kp_gpios[] __initdata = {
	PM8921_GPIO_INPUT(3, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(4, PM_GPIO_PULL_UP_30),
};

static struct pm8xxx_gpio_init pm8921_cdp_kp_gpios[] __initdata = {
	PM8921_GPIO_INPUT(27, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(42, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(17, PM_GPIO_PULL_UP_1P5),     /* SD_WP */
};

/* Initial PM8XXX MPP configurations */
static struct pm8xxx_mpp_init pm8xxx_mpps[] __initdata = {
	PM8921_MPP_DISABLE(1), /* N/C */
	PM8921_MPP_DISABLE(2), /* N/C */
	PM8921_MPP_DISABLE(3), /* N/C */
	PM8921_MPP_DISABLE(4), /* N/C */
	/* MPP 5 and 6 are handled by QCT*/
	/* External 5V regulator enable; used by USB_OTG switches. */
	PM8921_MPP_INIT(7, D_OUTPUT, PM8921_MPP_DIG_LEVEL_S4, DOUT_CTRL_LOW),
	/* BL_THERM */
	PM8921_MPP_INIT(8, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH5, DOUT_CTRL_LOW),
	/* RF_ID */
	PM8921_MPP_INIT(9, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH6, DOUT_CTRL_LOW),
	PM8921_MPP_DISABLE(10), /* N/C */
	PM8921_MPP_DISABLE(11), /* N/C */
	PM8921_MPP_DISABLE(12), /* N/C */

	/* TMM_ANT_SW1 */
	PM8821_MPP_INIT(1, D_OUTPUT, PM8821_MPP_DIG_LEVEL_1P8, DOUT_CTRL_HIGH),
	/* TMM_ANT_SW2 */
	PM8821_MPP_INIT(2, D_OUTPUT, PM8821_MPP_DIG_LEVEL_1P8, DOUT_CTRL_LOW),
	/* TMM_ANT_SW3 */
	PM8821_MPP_INIT(3, D_OUTPUT, PM8821_MPP_DIG_LEVEL_1P8, DOUT_CTRL_HIGH),
	PM8821_MPP_DISABLE(4), /* N/C */
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

	if (machine_is_apq8064_mtp())
		for (i = 0; i < ARRAY_SIZE(pm8921_mtp_kp_gpios); i++) {
			rc = pm8xxx_gpio_config(pm8921_mtp_kp_gpios[i].gpio,
						&pm8921_mtp_kp_gpios[i].config);
			if (rc) {
				pr_err("%s: pm8xxx_gpio_config: rc=%d\n",
					__func__, rc);
				break;
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
	.resume_voltage_delta	= 80,
	.resume_charge_percent	= 95,
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
	.btc_override           = 1,
	.btc_override_cold_degc = 5,
	.btc_override_hot_degc  = 55,
	.btc_delay_ms           = 10000,
	.btc_panic_if_cant_stop_chg	= 1,
	.stop_chg_upon_expiry	= 1,
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
#else
	.battery_type			= BATT_OEM,
#endif
	.r_sense_uohm			= 10000,
	.v_cutoff			= V_CUTOFF_MV,
	.i_test				= 1000,
	.max_voltage_uv			= MAX_VOLTAGE_MV * 1000,
	.rconn_mohm			= 30,
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

static struct pm8xxx_vibrator_platform_data
apq8064_pm8xxx_vibrator_pdata __devinitdata = {
	.initial_vibrate_ms = 0,
	.level_mV = 2700,
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
	.vibrator_pdata		= &apq8064_pm8xxx_vibrator_pdata,
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

	if (machine_is_apq8064_rumi3()) {
		apq8064_pm8921_irq_pdata.devirq = 0;
		apq8064_pm8821_irq_pdata.devirq = 0;
	}
}

static struct gpio_keys_button fusion3_keys[] = {
	{
		.code           = KEY_CAMERA_FOCUS,
		.gpio           = PM8921_GPIO_PM_TO_SYS(3),
		.desc           = "cam_af_key",
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
};

static struct gpio_keys_platform_data fusion3_keys_data = {
	.buttons        = fusion3_keys,
	.nbuttons       = ARRAY_SIZE(fusion3_keys),
};

static struct platform_device fusion3_kp_pdev = {
	.name           = "gpio-keys",
	.id             = -1,
	.dev            = {
		.platform_data  = &fusion3_keys_data,
	},
};

#define GPIO_SW_SIM_DETECTION		1

static struct gpio_event_direct_entry gpio_sw_gpio_map[] = {
	{PM8921_GPIO_PM_TO_SYS(GPIO_SW_SIM_DETECTION), SW_JACK_PHYSICAL_INSERT},
};

static struct gpio_event_input_info gpio_sw_gpio_info = {
	.info.func = gpio_event_input_func,
	.info.no_suspend = true,
	.flags = 0,
	.type = EV_SW,
	.keymap = gpio_sw_gpio_map,
	.keymap_size = ARRAY_SIZE(gpio_sw_gpio_map),
};

static struct gpio_event_info *pmic_keypad_info[] = {
	&gpio_sw_gpio_info.info,
};

struct gpio_event_platform_data pmic_keypad_data = {
	.name       = "sim-detection",
	.info       = pmic_keypad_info,
	.info_count = ARRAY_SIZE(pmic_keypad_info),
};

static struct platform_device pmic_keypad_device = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= 1,
	.dev	= {.platform_data = &pmic_keypad_data},
};

static int __init input_devices_init(void)
{
	platform_device_register(&fusion3_kp_pdev);
	platform_device_register(&pmic_keypad_device);
	return 0;
}

static void __exit input_devices_exit(void)
{
}

module_init(input_devices_init);
module_exit(input_devices_exit);

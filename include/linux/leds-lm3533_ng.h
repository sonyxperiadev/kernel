/*
 * /include/linux/leds-lm3533_ng.h
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * Author: Aleksej Makarov <aleksej.makarov@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. *
 */
#ifndef _LEDS_LM3533_H_
#define _LEDS_LM3533_H_

#define LM3533_DEV_NAME "lm3533"

/* Full scale current register value form current, max 29800 uA*/
#define MIN_FSC_UA 5000
#define FSC_UA_LSB 800
#define I_UA_TO_FSC(ua) (((ua) - MIN_FSC_UA) / FSC_UA_LSB)

/* ALS input current register value form current, max 1270 uA */
#define ALS_UA_LSB 10
#define ALS_CUR_UA_TO_REG(ua) ((ua) / ALS_UA_LSB)

enum lm3533_control_bank {
	LM3533_CBNKA,
	LM3533_CBNKB,
	LM3533_CBNKC,
	LM3533_CBNKD,
	LM3533_CBNKE,
	LM3533_CBNKF,
	LM3533_BANK_NUM,
};

enum lm3533_led {
	LM3533_HVLED1,
	LM3533_HVLED2,
	LM3533_LVLED1,
	LM3533_LVLED2,
	LM3533_LVLED3,
	LM3533_LVLED4,
	LM3533_LVLED5,
	LM3533_LED_NUM,
};

enum lm3533_pwm {
	LM3533_PWM_ENA = 1 << 0,
	LM3533_PWM_Z0  = 1 << 1,
	LM3533_PWM_Z1  = 1 << 2,
	LM3533_PWM_Z2  = 1 << 3,
	LM3533_PWM_Z3  = 1 << 4,
	LM3533_PWM_Z4  = 1 << 5,
	LM3533_PWM_ALL = LM3533_PWM_ENA | LM3533_PWM_Z0 | LM3533_PWM_Z1 |
			LM3533_PWM_Z2 | LM3533_PWM_Z3 | LM3533_PWM_Z4,
};

enum lm3533_br_ctl {
	/* only for HV BANK A: */
	LM3533_HVA_MAP_LIN  = 1 << 1,
	LM3533_HVA_MAP_EXP  = 0 << 1,
	LM3533_HVA_ALS1     = 1 << 0,
	LM3533_HVA_BR_CTL   = 0 << 0,

	/* only for HV BANK B: */
	LM3533_HVB_MAP_LIN  = 1 << 3,
	LM3533_HVB_MAP_EXP  = 0 << 3,
	LM3533_HVB_ALS2     = 1 << 2,
	LM3533_HVB_BR_CTL   = 0 << 2,

	/* only for LV BANKS: */
	LM3533_LV_BR_CTL   = 0 << 0,
	LM3533_LV_ALS2     = 2 << 0,
	LM3533_LV_ALS3     = 3 << 0,
	LM3533_LV_MAP_LIN  = 1 << 2,
	LM3533_LV_MAP_EXP  = 0 << 2,
	LM3533_LV_PTRN_ON  = 1 << 3,
	LM3533_LV_PTRN_OFF = 0 << 3,
};

enum lm3533_cpump_config {
	LM3533_PUMP_DIS       = 1 << 0,
	LM3533_PUMP_ENA       = 0 << 0,
	LM3533_PUMP_GAIN_AUTO = 0 << 1,
	LM3533_PUMP_GAIN_1X   = 2 << 1,
	LM3533_PUMP_GAIN_2X   = 3 << 1,
};


enum lm3533_ovp_boost_pwm_config {
	LM3533_BOOST_500KHZ  = 0 << 0,
	LM3533_BOOST_1000KHZ = 1 << 0,
	LM3533_OVP_16V  = 0 << 1,
	LM3533_OVP_24V  = 1 << 1,
	LM3533_OVP_32V  = 2 << 1,
	LM3533_OVP_40V  = 3 << 1,
	LM3533_PWM_LOW  = 0 << 3,
	LM3533_PWM_HIGH = 1 << 3,
};

enum lm3533_ramp_rate {
	LM3533_RR_2048US,
	LM3533_RR_262MS,
	LM3533_RR_524MS,
	LM3533_RR_1049MS,
	LM3533_RR_2097MS,
	LM3533_RR_4194MS,
	LM3533_RR_8389MS,
	LM3533_RR_16780MS,
};

enum lm3533_als_avg_time {
	LM3533_ALS_17920   = 0 << 3,
	LM3533_ALS_35840   = 1 << 3,
	LM3533_ALS_71680   = 2 << 3,
	LM3533_ALS_143360  = 3 << 3,
	LM3533_ALS_286720  = 4 << 3,
	LM3533_ALS_573440  = 5 << 3,
	LM3533_ALS_1146880 = 6 << 3,
	LM3533_ALS_2293760 = 7 << 3,
};

enum lm3533_als_control {
	LM3533_ALS_FAST_START = 1 << 2,
	LM3533_ALS_ANALOG     = 0 << 1,
	LM3533_ALS_DIGITAL    = 1 << 1,
};

enum lm3533_als_algo {
	LM3533_ALS1_DIRECT = 0 << 0,
	LM3533_ALS1_UPONLY = 1 << 0,
	LM3533_ALS1_DDELAY = 3 << 0,
	LM3533_ALS2_DIRECT = 0 << 2,
	LM3533_ALS2_UPONLY = 1 << 2,
	LM3533_ALS2_DDELAY = 3 << 2,
	LM3533_ALS3_DIRECT = 0 << 4,
	LM3533_ALS3_UPONLY = 1 << 4,
	LM3533_ALS3_DDELAY = 3 << 4,
	LM3533_SCALER_DIRECT = 0 << 6,
	LM3533_SCALER_UPONLY = 1 << 6,
	LM3533_SCALER_DDELAY = 3 << 6,
};

enum lm3533_led_fault {
	LM3533_OPEN_DISABLED  = 0 << 0,
	LM3533_OPEN_ENABLED   = 1 << 0,
	LM3533_SHORT_DISABLED = 0 << 1,
	LM3533_SHORT_ENABLED  = 1 << 1,
};

struct lm3533_bank_config {
	u8 pwm; /* ORed from enum lm3533_pwm    */
	u8 ctl; /* ORed from enum lm3533_br_ctl */
	u8 fsc; /* use I_UA_TO_FSC(ua)          */
	const char *iname; /* interface name */
};

struct lm3533_led_config {
	bool connected;
	bool cpout;
	enum lm3533_control_bank bank;
};

struct lm3533_startup_brightenss {
	char const *intf_name;
	u8 brightness;
};

struct lm3533_platform_data {
	struct lm3533_bank_config b_cnf[LM3533_BANK_NUM];
	struct lm3533_led_config l_cnf[LM3533_LED_NUM];
	u8 cpump_cnf; /* ORed from enum lm3533 cpump_config */
	u8 ovp_boost_pwm; /* ORed from enum lm3533_ovp_boost_pwm_config */
	u8 led_fault; /* ORed from enum lm3533_led_fault */
	u8 als_control; /* ORed from enums lm3533_als_avg_time and
	lm3533_als_control*/
	u8 als_algo; /* ORed from enum llm3533_als_algo */
	u8 als_input_current; /* use ALS_CUR_UA_TO_REG(ua) */
	u8 als_down_delay; /* in number ov avg periods */
	enum lm3533_ramp_rate runtime_rr;
	enum lm3533_ramp_rate startup_rr;
	int (*setup)(struct device *dev);
	void (*teardown)(struct device *dev);
	int (*power_on)(struct device *dev);
	int (*power_off)(struct device *dev);
	int (*als_on)(struct device *dev);
	int (*als_off)(struct device *dev);
	/* pointer to NULL-terminated array */
	struct lm3533_startup_brightenss *startup_brightness;
};

#endif

/*
 *  Copyright (C) 2012, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef __MXT_H__
#define __MXT_H__

#define MXT_DEV_NAME "Atmel MXT224S"

#define MXT_1664S_SW_RESET_TIME		300	/* msec */
#define MXT_1664S_HW_RESET_TIME		300	/* msec */
#define MXT_1664S_FW_RESET_TIME		500	/* msec */

#define MXT_CONFIG(x, y) {.instance = y, .size = ARRAY_SIZE(x##_config)-1,\
			  .data = x##_config}
#define MXT_CONFIG_TA(x, y) {.instance = y, \
		.size = ARRAY_SIZE(x##_config_ta)-1,\
		.data = x##_config_ta}

enum {
	RESERVED_T0 = 0,
	RESERVED_T1,
	DEBUG_DELTAS_T2,
	DEBUG_REFERENCES_T3,
	DEBUG_SIGNALS_T4,
	GEN_MESSAGEPROCESSOR_T5,
	GEN_COMMANDPROCESSOR_T6,
	GEN_POWERCONFIG_T7,
	GEN_ACQUISITIONCONFIG_T8,
	TOUCH_MULTITOUCHSCREEN_T9,
	TOUCH_SINGLETOUCHSCREEN_T10,
	TOUCH_XSLIDER_T11,
	TOUCH_YSLIDER_T12,
	TOUCH_XWHEEL_T13,
	TOUCH_YWHEEL_T14,
	TOUCH_KEYARRAY_T15,
	PROCG_SIGNALFILTER_T16,
	PROCI_LINEARIZATIONTABLE_T17,
	SPT_COMCONFIG_T18,
	SPT_GPIOPWM_T19,
	PROCI_GRIPFACESUPPRESSION_T20,
	RESERVED_T21,
	PROCG_NOISESUPPRESSION_T22,
	TOUCH_PROXIMITY_T23,
	PROCI_ONETOUCHGESTUREPROCESSOR_T24,
	SPT_SELFTEST_T25,
	DEBUG_CTERANGE_T26,
	PROCI_TWOTOUCHGESTUREPROCESSOR_T27,
	SPT_CTECONFIG_T28,
	SPT_GPI_T29,
	SPT_GATE_T30,
	TOUCH_KEYSET_T31,
	TOUCH_XSLIDERSET_T32,
	RESERVED_T33,
	GEN_MESSAGEBLOCK_T34,
	SPT_GENERICDATA_T35,
	RESERVED_T36,
	DEBUG_DIAGNOSTIC_T37,
	SPT_USERDATA_T38,
	SPARE_T39,
	PROCI_GRIPSUPPRESSION_T40,
	SPARE_T41,
	PROCI_TOUCHSUPPRESSION_T42,
	SPT_DIGITIZER_T43,
	SPARE_T44,
	SPARE_T45,
	SPT_CTECONFIG_T46,
	PROCI_STYLUS_T47,
	PROCG_NOISESUPPRESSION_T48,
	SPARE_T49,
	SPARE_T50,
	SPARE_T51,
	TOUCH_PROXIMITY_KEY_T52,
	GEN_DATASOURCE_T53,
	SPARE_T54,
	ADAPTIVE_T55,
	PROCI_SHIELDLESS_T56,
	PROCI_EXTRATOUCHSCREENDATA_T57,
	SPARE_T58,
	SPARE_T59,
	SPARE_T60,
	SPT_TIMER_T61,
	PROCG_NOISESUPPRESSION_T62,
	PROCI_ACTIVESTYLUS_T63,
	RESERVED_T255 = 255,
};

struct mxt_config_struct {
	u8 type;
	u8 size;
	u8 instance;
	u8 *data;
};


struct mxt_platform_data {
	int num_of_fingers;
	int gpio_read_done;
	struct mxt_config_struct *config;
	struct mxt_config_struct *ta_config;
	const char *config_ver;
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int min_z;
	int max_z;
	int min_w;
	int max_w;
	u8 exit_flag;
	int (*setup_power)(struct device *, bool);
	void (*power_con) (bool);
	u8 boot_address;
	u8 *touchscreen_is_pressed;
	const char *firmware_name;
	u8 *initial_cable_status;
};

#define SEC_FAC_TSP

#if defined(SEC_FAC_TSP)
#define TSP_CMD(name, func) .cmd_name = name, .cmd_func = func
#define TOSTRING(x) (#(x))
extern struct class *sec_class;
#endif

#endif	/* __MXT_H__ */

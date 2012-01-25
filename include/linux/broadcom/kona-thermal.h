/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/broadcom/kona-thermal.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
* kona-thermal.h
*
* PURPOSE:
*
*
*
* NOTES:
*
* ****************************************************************************/

#ifndef __KONA_THERMAL_H__
#define __KONA_THERMAL_H__

typedef enum {
	SENSOR_BB_TMON,		/*Kona TMON for baseband temp monitoring */
	SENSOR_BB_MULTICORE,	/*allow for multi-core */
	SENSOR_PMU,
	SENSOR_BATTERY,
	SENSOR_CRYSTAL,		/* 32KHz, 26Mhz */
	SENSOR_PA,
	SENSOR_OTHER		/* allow for growth */
} sensor_type;

typedef enum {
	SENSOR_READ_DIRECT,	/* Direct register access */
	SENSOR_READ_PMU_I2C,	/* I2C access           */
	SENSOR_READ_SPI		/* SPI access           */
} sensor_read_type;

struct temp_threshold {
	unsigned temp_low;	/*undertemp  threshold */
	unsigned temp_high;	/*overtemp  threshold */
};

typedef enum {
	WARNING_1,		/* current enum allows two warning and one fatal alarm */
	WARNING_2,		/* threshold but this can be expanded , if necessary */
	FATAL
} threshold_type;

typedef enum {
	LOW,			/* this is used for setting the undertemp threshold     */
	HIGH			/* this is used for setting the overtemp threshold      */
} temperature_type;

typedef enum {
	THERM_ACTION_NONE,
	THERM_ACTION_NOTIFY,
	THERM_ACTION_NOTIFY_SHUTDOWN,
	THERM_ACTION_SHUTDOWN
} therm_action_type;

typedef enum {
	SENSOR_PERIODIC_READ,	/*Interval located in separate field            */
	SENSOR_INTERRUPT,	/*Interrupt detection of temperature violation  */
	SENSOR_DRX_SYNC		/* synchronize sensor reading to DRX cycles     */
} sensor_control_type;

struct thermal_sensor_config {
	unsigned thermal_id;
	char *thermal_name;
	sensor_type thermal_type;
	unsigned thermal_mc;
	sensor_read_type thermal_read;
	unsigned thermal_location;
	int long thermal_warning_lvl_1;
	int long thermal_warning_lvl_2;
	int long thermal_fatal_lvl;
	therm_action_type thermal_warning_action;
	therm_action_type thermal_fatal_action;
	unsigned thermal_sensor_param;
	sensor_control_type thermal_control;
	unsigned *convert_callback;
};
typedef struct thermal_sensor_config THERMAL_SENSOR_CONFIG_t;

struct therm_data {
	unsigned flags;
	int thermal_update_interval;
	unsigned num_sensors;
	struct thermal_sensor_config *sensors;
};
#endif

unsigned int thermal_get_adc(int adc_sig);

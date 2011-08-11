/*
 * csapi_adc.h - ADC chipset API
 *
 * Contact: aliaksei.katovich@nokia.com
 *
 * Copyright (C) 2011 Nokia Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef __CSAPI_ADC_H__
#define __CSAPI_ADC_H__

#define CSAPI_ADC_H_MAJOR 1
#define CSAPI_ADC_H_MINOR 0

enum csapi_adc_channel {
	CSAPI_ADC_VBAT	= 1,	/* battery voltage (scaled)		*/
	CSAPI_ADC_VCHAR,	/* charger voltage			*/
	CSAPI_ADC_ICHAR,	/* charger current			*/
	CSAPI_ADC_BSI,		/* battery size identification		*/
	CSAPI_ADC_BTEMP,	/* battery temperature			*/
	CSAPI_ADC_PATEMP,	/* power amplifier temperature		*/
	CSAPI_ADC_IBAT_AVG,	/* battery current (average)		*/
	CSAPI_ADC_IBAT,		/* battery current (one period)		*/
	CSAPI_ADC_IBAT_CC,	/* battery coulomb counter		*/
	CSAPI_ADC_VBACK,	/* backup battery voltage		*/
	CSAPI_ADC_VBUS,		/* VBUS voltage				*/
	CSAPI_ADC_IDIN,		/* ACA and float legacy ID level	*/
	CSAPI_ADC_ALS,		/* ambient light sensor			*/
	CSAPI_ADC_CTEMP,	/* crystal temperature			*/
	CSAPI_ADC_MAIN_CAL,	/* main ADC channel			*/
	CSAPI_ADC_VENDOR_CH_0,	/* vendor specific channels start here	*/
	CSAPI_ADC_VENDOR_CH_1,
	CSAPI_ADC_VENDOR_CH_2,
	CSAPI_ADC_VENDOR_CH_3,
	CSAPI_ADC_VENDOR_CH_4,
	CSAPI_ADC_VENDOR_CH_5,
	CSAPI_ADC_VENDOR_CH_6,
	CSAPI_ADC_VENDOR_CH_7,
	CSAPI_ADC_VENDOR_CH_8,
	CSAPI_ADC_VENDOR_CH_9,
	CSAPI_ADC_VENDOR_CH_10,
	CSAPI_ADC_VENDOR_CH_11,
	CSAPI_ADC_VENDOR_CH_12,
	CSAPI_ADC_VENDOR_CH_13,
	CSAPI_ADC_VENDOR_CH_14,
	CSAPI_ADC_VENDOR_CH_15,
};

/**
 * Chipset API client reference structure.
 *
 * Linux kernel: normally structure is accessed by platform driver during
 * probe call. Clients themselves are specified in platform data file.
 *
 * Though clients handling is vendor specific each implementation should
 * nevertheless be agreed with API holder.
 */
struct csapi_cli;

/**
 * Callback function to handle asynchronous data requests.
 *
 * @cha		requested channel
 * @val		returned value
 * @err		appropriate negative errno if request failed
 * @ptr		optional private data pointer
 */
typedef void (*csapi_cb) (u8 cha, int val, int err, void *ptr);

/**
 * Read raw value from given ADC channel.
 *
 * @cli		chipset API client reference
 * @cha		channel to read
 * @val		returned value
 * @cb		optional asynchronous result callback
 * @ptr		optional private data pointer specified by caller and passed to
 *		callback function later on
 *
 * @return	appropriate negative errno if failed
 *
 * Function should not fail if callback is not specified but rather become
 * synchronous.
 */
int csapi_adc_raw_read(struct csapi_cli *cli,
		       u8 cha, u32 *val, csapi_cb cb, void *ptr);

/**
 * Read unit value from given ADC channel.
 *
 * @cli		chipset API client reference
 * @cha		channel to read
 * @val		returned value
 * @cb		optional asynchronous result callback
 * @ptr		optional private data pointer specified by caller and passed to
 *		callback function later on
 *
 * @return	appropriate negative errno if failed
 *
 * Function should not fail if callback is not specified but rather become
 * synchronous.
 */
int csapi_adc_unit_read(struct csapi_cli *cli,
			u8 cha, u32 *val, csapi_cb cb, void *ptr);

/**
 * Convert raw reading into unit value for given ADC channel.
 *
 * @cli		chipset API client reference
 * @cha		channel to convert
 * @val		raw value to convert
 *
 * @return	unit value
 */
int csapi_adc_unit_convert(struct csapi_cli *cli, u8 cha, u32 raw);

/**
 * Convert raw reading into unit value for given ADC channel and adjust for
 * calibration.
 *
 * @cli		chipset API client reference
 * @cha		channel to convert
 * @raw		raw value to convert
 *
 * @return	appropriate negative errno if failed
 */
int csapi_cal_unit_convert(struct csapi_cli *cli, u8 cha, u32 raw);

/**
 * Disable channel conversion and return locked unit value on subsequent
 * readings.
 *
 * @cli		chipset API client reference
 * @cha		channel to lock
 * @val		unit value to lock
 *
 * @return	appropriate negative errno if failed
 */
int csapi_cal_unit_convert_lock(struct csapi_cli *cli, u8 cha, int val);

/**
 * Enable channel conversion
 *
 * @cli		chipset API client reference
 * @cha		channel to unlock
 */
int csapi_cal_unit_convert_unlock(struct csapi_cli *cli, u8 cha);

/**
 * Get calibration parameters for given ADC channel.
 *
 * @cli		chipset API client reference
 * @cha		requested channel
 * @id		calibration data identifier
 * @p1		vendor specific calibration parameter #1
 * @p2		vendor specific calibration parameter #2
 * @p3		vendor specific calibration parameter #3
 *
 * @return	appropriate negative errno if failed
 */
int csapi_cal_data_get(struct csapi_cli *cli,
		       u8 cha, u32 *id, u32 *p1, u32 *p2, u32 *p3);

/**
 * Set calibration parameters for given ADC channel.
 *
 * @cli		chipset API client reference
 * @cha		requsted channel
 * @id		calibration data identifier
 * @p1		vendor specific calibration parameter #1
 * @p2		vendor specific calibration parameter #2
 * @p3		vendor specific calibration parameter #3
 *
 * @return	appropriate negative errno if failed
 */
int csapi_cal_data_set(struct csapi_cli *cli,
		       u8 cha, u32 id, u32 p1, u32 p2, u32 p3);


enum csapi_cal_type {
	CSAPI_CAL_APPLIED_VOLTAGE = 1,
	CSAPI_CAL_APPLIED_CURRENT,
	CSAPI_CAL_SENSOR,
};

struct csapi_cal_data {
	enum csapi_cal_type	type;	/* adc calibration type */
	u32			id;	/* data identifier */
	u8			cha;	/* requested channel */
	u32			raw;	/* raw value */
	int			ref;	/* reference value */
};

struct csapi_cal_req {
	size_t			size;	/* number of data elements */
	struct csapi_cal_data	data[];
};

/**
 * Perform calibration calculations.
 *
 * @req		calibration request
 *
 * @return	appropriate negative errno if failed
 */
int csapi_cal_calc(struct csapi_cal_req *req);

#endif /* __CSAPI_ADC_H__ */

/*
 * smb380.h - SMB380 Tri-axis accelerometer driver
 *
 * Copyright (c) 2010 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as 
 * published by the Free Software Foundation.
 *
 */

#ifndef _SMB380_H_
#define _SMB380_H_

enum scale_range {
	RANGE_2G,
	RANGE_4G,
	RANGE_8G,
};

/* Used to setup the digital filtering bandwidth of ADC output */
enum filter_bw {
	BW_25HZ,
	BW_50HZ,
	BW_100HZ,
	BW_190HZ,
	BW_375HZ,
	BW_750HZ,
	BW_1500HZ,
};

struct smb380_platform_data {
	enum scale_range range;
	enum filter_bw bandwidth;
	u8 enable_adv_int;
	u8 new_data_int;
	u8 hg_int;
	u8 lg_int;
	u8 lg_dur;
	u8 lg_thres;
	u8 lg_hyst;
	u8 hg_dur;
	u8 hg_thres;
	u8 hg_hyst;
	u8 any_motion_dur;
	u8 any_motion_thres;
	u8 any_motion_int;
};

#endif /* _SMB380_H_ */

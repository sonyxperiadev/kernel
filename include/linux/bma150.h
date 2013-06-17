/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL"). 
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef _BMA150_H_
#define _BMA150_H_

#define BMA150_DRIVER		"bma150"

#define BMA150_RANGE_2G		0
#define BMA150_RANGE_4G		1
#define BMA150_RANGE_8G		2

#define BMA150_BW_25HZ		0
#define BMA150_BW_50HZ		1
#define BMA150_BW_100HZ		2
#define BMA150_BW_190HZ		3
#define BMA150_BW_375HZ		4
#define BMA150_BW_750HZ		5
#define BMA150_BW_1500HZ	6

struct bma150_cfg {
	bool any_motion_int;		/* Set to enable any-motion interrupt */
	bool hg_int;			/* Set to enable high-G interrupt */
	bool lg_int;			/* Set to enable low-G interrupt */
	unsigned char any_motion_dur;	/* Any-motion duration */
	unsigned char any_motion_thres;	/* Any-motion threshold */
	unsigned char hg_hyst;		/* High-G hysterisis */
	unsigned char hg_dur;		/* High-G duration */
	unsigned char hg_thres;		/* High-G threshold */
	unsigned char lg_hyst;		/* Low-G hysterisis */
	unsigned char lg_dur;		/* Low-G duration */
	unsigned char lg_thres;		/* Low-G threshold */
	unsigned char range;		/* one of BMA0150_RANGE_xxx */
	unsigned char bandwidth;	/* one of BMA0150_BW_xxx */
};

struct bma150_platform_data {
	struct bma150_cfg cfg;
	int (*irq_gpio_cfg)(void);
};

#endif /* _BMA150_H_ */

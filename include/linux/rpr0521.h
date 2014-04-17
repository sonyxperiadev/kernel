/*****************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
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

/*
 * Definitions for rohm rpr-0521 ambient light and proximity sensor chip.
 */

#ifndef _RPR0521_H_
#define _RPR0521_H_

#define RPR0521_PROX_AUTO_CALIBRATION	0
#define RPR0521_PROX_USER_CALIBRATION	1

#define MASK_LONG	(0xFFFFFFFF)
#define MASK_CHAR	(0xFF)
#define CUT_UNIT	(10)
struct CALC_DATA {
	unsigned long long data;
	unsigned long long data0;
	unsigned long long data1;
	unsigned char      gain_data0;
	unsigned char      gain_data1;
	unsigned long      dev_unit;
	unsigned char      als_time;
	unsigned short     als_data0;
	unsigned short     als_data1;
};

struct CALC_ANS {
	unsigned long positive;
	unsigned long decimal;
};

#endif /* _RPR0521_H_ */

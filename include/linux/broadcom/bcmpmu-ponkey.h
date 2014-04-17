/*
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

/*******************************************************************************
 * Copyright 2010 Broadcom Corporation.  All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under th
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 * ****************************************************************************/


#ifndef __BCMPMU_PONKEY_H__
#define __BCMPMU_PONKEY_H__

#define BCM59039_CO_DIG_REV 3
#define BCM59039_CO_ANA_REV 3
#define BCM59042_CO_DIG_REV 3
#define BCM59042_CO_ANA_REV 3

enum {
	POK_HLD_DEB_0 = 0,
	POK_HLD_DEB_10MS,
	POK_HLD_DEB_50MS,
	POK_HLD_DEB_100MS,
	POK_HLD_DEB_500MS,
	POK_HLD_DEB_1000MS,
	POK_HLD_DEB_2000MS,
	POK_HLD_DEB_4000MS
};

enum {
	POK_SHTDWN_DLY_0SEC = 0,
	POK_SHTDWN_DLY_1SEC,
	POK_SHTDWN_DLY_2SEC,
	POK_SHTDWN_DLY_3SEC,
	POK_SHTDWN_DLY_4SEC,
	POK_SHTDWN_DLY_5SEC,
	POK_SHTDWN_DLY_6SEC,
	POK_SHTDWN_DLY_7SEC,
	POK_SHTDWN_DLY_8SEC,
	POK_SHTDWN_DLY_9SEC,
	POK_SHTDWN_DLY_10SEC,
	POK_SHTDWN_DLY_11SEC,
	POK_SHTDWN_DLY_12SEC,
	POK_SHTDWN_DLY_13SEC,
	POK_SHTDWN_DLY_14SEC,
	POK_SHTDWN_DLY_15SEC
};

enum {
	POK_RESTRT_DEB_0SEC = 0,
	POK_RESTRT_DEB_1SEC,
	POK_RESTRT_DEB_2SEC,
	POK_RESTRT_DEB_3SEC,
	POK_RESTRT_DEB_4SEC,
	POK_RESTRT_DEB_5SEC,
	POK_RESTRT_DEB_6SEC,
	POK_RESTRT_DEB_7SEC,
	POK_RESTRT_DEB_8SEC,
	POK_RESTRT_DEB_9SEC,
	POK_RESTRT_DEB_10SEC,
	POK_RESTRT_DEB_11SEC,
	POK_RESTRT_DEB_12SEC,
	POK_RESTRT_DEB_13SEC,
	POK_RESTRT_DEB_14SEC,
	POK_RESTRT_DEB_15SEC
};

enum {
	POK_RESTRT_DLY_P5SEC = 0,
	POK_RESTRT_DLY_1SEC,
	POK_RESTRT_DLY_2SEC,
	POK_RESTRT_DLY_4SEC
};

#endif


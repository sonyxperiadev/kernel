/******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/regulator/tps728xx.h
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
* tps728xx.h
*
* PURPOSE:
*
*
*
* NOTES:
*
* ****************************************************************************/

#ifndef __TPS728XX_REGULATOR_H__
#define __TPS728XX_REGULATOR_H__

struct tps728xx_plat_data {
	int gpio_vset;
	int gpio_en;
	int vout0;
	int vout1;

	struct regulator_init_data *initdata;
};


#endif

/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/broadcom/bcm59055-power.h
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
* bcm59055-power.h
*
* PURPOSE:
*
*
*
* NOTES:
*
* ****************************************************************************/


#ifndef __BCM59055_POWER_H__
#define __BCM59055_POWER_H__

enum {
	CURRENT_100_MA = 0,
	CURRENT_350_MA,
	CURRENT_500_MA,
	CURRENT_600_MA,
	CURRENT_800_MA,
	CURRENT_900_MA,
	CURRENT_900_1_MA,
	CURRENT_900__2_MA,
	CURRENT_MAX
};

enum {
	VFLOAT_3_6_V,
	VFLOAT_3_9_V,
	VFLOAT_4_0_V,
	VFLOAT_4_04_V,
	VFLOAT_4_1_V,
	VFLOAT_4_125_V,
	VFLOAT_4_15_V,
	VFLOAT_4_175_V,
	VFLOAT_4_2_V,
	VFLOAT_4_225_V,
	VFLOAT_4_25_V,
	VFLOAT_4_275_V,
	VFLOAT_4_3_V,
	VFLOAT_4_325_V,
	VFLOAT_4_35_V,
	VFLOAT_4_375_V,
	VFLOAT_MAX
};


/* Env1 BIT Def */
#define P_MBWV				(0x1)
#define P_MBWV_DELTA		(0x1 << 1)
#define P_ERC				(0x1 << 2)
#define P_MBMC				(0x1 << 3)
#define P_MBOV				(0x1 << 4)
#define P_MBUV				(0x1 << 5)
#define P_BSIWV				(0x1 << 6)
#define P_BBLOWB			(0x1 << 7)

/* Env2 BIT Def */
#define P_CGPD_ENV			(0x1)
#define P_UBPD_ENV			(0x1 << 1)
#define P_UBPD_USBDET		(0x1 << 2)
#define P_UBPD_INT			(0x1 << 3)
#define P_CGPD_PRI			(0x1 << 4)
#define P_UBPD_PRI			(0x1 << 5)
#define WAC_VALID			(0x1 << 6)
#define USB_VALID			(0x1 << 7)

/* Env3 BIT Def */
#define P_CGPD_CHG			(0x1)
#define P_UBPD_CHG			(0x1 << 1)
#define P_CGMBC				(0x1 << 2)
#define P_UBMBC				(0x1 << 3)
#define P_CHGOV				(0x1 << 4)
#define P_USBOV				(0x1 << 5)
#define ACP7_DET			(0x1 << 6)
#define USB_PORT_DISABLE	(0x1 << 7)

#define USB_HOSTEN			(0x1 << 1)
#define WAC_HOSTEN			(0x1)
#define USB_FC_CC_MASK		(0x7)
#define WAC_FC_CC_MASK		(0x7)
#define VFLOAT_MASK			(0xF)
#define VFLOATMAX_LOCK		(0x1 << 4)
#define ICCMAX_MASK			(0x7)
#define ICCMAX_LOCK			(0x1 << 3)
#define SWUP				(0x1)


#endif
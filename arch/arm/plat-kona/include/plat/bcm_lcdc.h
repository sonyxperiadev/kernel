/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/bcm_lcdc.h
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
*  bcm_lcdc.h
*
*  PURPOSE:
*
*     This file contains platform data structures for LCD controller.
*
*  NOTES:
*
*****************************************************************************/

#ifndef __PLAT_BCM_LCDC_H
#define __PLAT_BCM_LCDC_H

struct lcdc_platform_data_t {
	int gpio;
	bool te_supported;
};

#endif /* __PLAT_BCM_LCDC_H */

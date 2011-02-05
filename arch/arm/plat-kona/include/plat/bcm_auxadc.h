/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/bcm_auxadc.h
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
/**
 *
 *   @file   bcm_auxadc.h
 *   @brief  Broadcom platform header for AUX ADC modules.
 *
 ****************************************************************************/
/**
*   @defgroup   AuxADCAPIGroup   Auxilliary Analog-to-Digital Converter API's
*   @brief      This group defines the Aux ADC group API's
*
*****************************************************************************/

#ifndef __BCM_AUXADC__
#define __BCM_AUXADC__

/** @addtogroup AuxADCAPIGroup
	@{
*/

/**
* Provide access to specified ADC channel.
*
* @param regID - Channel number to access
*
* @return ADC output
*/
extern int auxadc_access(int regID);

/** @} */

struct bcm_plat_auxadc {
	u16 readmask;
	int ready;
	int start;
	u16 croff;
	u16 rdoff;
	int auxpm;
	int regoff;
	int bgoff;
};
#endif /*__BCM_AUXADC__*/

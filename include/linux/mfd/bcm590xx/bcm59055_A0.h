/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#ifndef __LINUX_MFD_BCM59055_A0_H
#define __LINUX_MFD_BCM59055_A0_H

#include <linux/mfd/bcm590xx/pmic.h> 

/* Register addresses */
#define BCM59055_REG_RFOPMODCTRL      0xA0    /* R/W  RFLDO */
#define BCM59055_REG_CAMOPMODCTRL     0xA1    /* R/W  CAMLDO */
#define BCM59055_REG_HV1OPMODCTRL     0xA2    /* R/W  HV1LDO */
#define BCM59055_REG_HV2OPMODCTRL     0xA3    /* R/W  HV2LDO */
#define BCM59055_REG_HV3OPMODCTRL     0xA4    /* R/W  HV3LDO */
#define BCM59055_REG_HV4OPMODCTRL     0xA5    /* R/W  HV4LDO */
#define BCM59055_REG_HV5OPMODCTRL     0xA6    /* R/W  HV5LDO */
#define BCM59055_REG_HV6OPMODCTRL     0xA7    /* R/W  HV6LDO */
#define BCM59055_REG_HV7OPMODCTRL     0xA8    /* R/W  HV7LDO */
#define BCM59055_REG_SIMOPMODCTRL     0xA9    /* R/W  SIMLDO */
#define BCM59055_REG_CSROPMODCTRL     0xAA    /* R/W  CSRLDO */
#define BCM59055_REG_IOSROPMODCTRL    0xAB    /* R/W  IOSRLDO */
#define BCM59055_REG_SDSROPMODCTRL    0xAC    /* R/W  SDSRLDO */

#define BCM59055_REG_RFLDOCTRL        0xB0
#define BCM59055_REG_CAMLDOCTRL       0xB1
#define BCM59055_REG_HVLDO1CTRL       0xB2
#define BCM59055_REG_HVLDO2CTRL       0xB3
#define BCM59055_REG_HVLDO3CTRL       0xB4
#define BCM59055_REG_HVLDO4CTRL       0xB5
#define BCM59055_REG_HVLDO5CTRL       0xB6
#define BCM59055_REG_HVLDO6CTRL       0xB7
#define BCM59055_REG_HVLDO7CTRL       0xB8
#define BCM59055_REG_SIMLDOCTRL       0xB9  
#define BCM59055_REG_CSRCTRL1         0xC0    
#define BCM59055_REG_IOSRCTRL1        0xC8    
#define BCM59055_REG_SDSRCTRL1        0xD0    

#define BCM59055_REG_PMUID            0x04    // 0x00    /* R    revision/id */

/* #defines to identify LDOs inside linux frameworks. */
#define BCM59055_RFLDO  0 
#define BCM59055_CAMLDO 1
#define BCM59055_HV1LDO 2
#define BCM59055_HV2LDO 3
#define BCM59055_HV3LDO 4
#define BCM59055_HV4LDO 5
#define BCM59055_HV5LDO 6
#define BCM59055_HV6LDO 7
#define BCM59055_HV7LDO 8
#define BCM59055_SIMLDO 9
#define BCM59055_CSR    10
#define BCM59055_IOSR   11
#define BCM59055_SDSR   12

#define PC2_IS_0_PC1_IS_0  0
#define PC2_IS_0_PC1_IS_1  2
#define PC2_IS_1_PC1_IS_0  4
#define PC2_IS_1_PC1_IS_1  6

#define EN_DIS_SHIFT    PC2_IS_1_PC1_IS_1 

struct bcm590xx_regulator_init_data ; 

void bcm59055_register_details(void **reg_info) ;
void bcm59055_regulator_desc(void **info) ;

void bcm59055_reg_init_dev_init(struct bcm590xx *bcm590xx)  ;

#endif

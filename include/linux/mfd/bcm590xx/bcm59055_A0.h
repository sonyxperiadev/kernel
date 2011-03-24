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

/* Interrupt register addresses. */
#define BCM59055_REG_INT1             0x30 
#define BCM59055_REG_INT2             0x31 
#define BCM59055_REG_INT3             0x32 
#define BCM59055_REG_INT4             0x33 
#define BCM59055_REG_INT5             0x34 
#define BCM59055_REG_INT6             0x35 
#define BCM59055_REG_INT7             0x36 
#define BCM59055_REG_INT8             0x37 
#define BCM59055_REG_INT9             0x38 
#define BCM59055_REG_INT10             0x39 
#define BCM59055_REG_INT11             0x3a 
#define BCM59055_REG_INT12             0x3b 
#define BCM59055_REG_INT13             0x3c 
#define BCM59055_REG_INT14             0x3d 

/* Interrupt mask register addresses. */
#define BCM59055_REG_INT1MSK             0x40 
#define BCM59055_REG_INT2MSK             0x41 
#define BCM59055_REG_INT3MSK             0x42 
#define BCM59055_REG_INT4MSK             0x43 
#define BCM59055_REG_INT5MSK             0x44 
#define BCM59055_REG_INT6MSK             0x45 
#define BCM59055_REG_INT7MSK             0x46 
#define BCM59055_REG_INT8MSK             0x47 
#define BCM59055_REG_INT9MSK             0x48 
#define BCM59055_REG_INT10MSK            0x49 
#define BCM59055_REG_INT11MSK            0x4a 
#define BCM59055_REG_INT12MSK            0x4b 
#define BCM59055_REG_INT13MSK            0x4c 
#define BCM59055_REG_INT14MSK            0x4d 

/* MBCCTRL register addresses */
#define BCM59055_REG_MBCCTRL1            0x50
#define BCM59055_REG_MBCCTRL2            0x51
#define BCM59055_REG_MBCCTRL3            0x52
#define BCM59055_REG_MBCCTRL4            0x53
#define BCM59055_REG_MBCCTRL5            0x54
#define BCM59055_REG_MBCCTRL6            0x55
#define BCM59055_REG_MBCCTRL7            0x56
#define BCM59055_REG_MBCCTRL8            0x57
#define BCM59055_REG_MBCCTRL9            0x58
#define BCM59055_REG_MBCCTRL10           0x59
#define BCM59055_REG_MBCCTRL11           0x5a

/* Register addresses */
#define BCM59055_REG_HOSTCTRL1        0x01
#define BCM59055_REG_PONKEYCTRL1      0x0C
#define BCM59055_REG_PONKEYCTRL2      0x0D
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

#define BCM59055_REG_ADCCTRL3       0x82
#define BCM59055_REG_ADCCTRL4       0x83

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

#define BCM59055_MAX_INT_REGS            14
#define BCM59055_MAX_INTMASK_REGS        14

#define BCM59055_INT_REG_BASE            BCM59055_REG_INT1
#define BCM59055_INT_MASK_REG_BASE       BCM59055_REG_INT1MSK

#define BCM59055_INVALID_ADCVAL              0x04 
#define BCM59055_REG_ADCCTRL3_VALID_BITS     0x03 

#define BCM59055_REG_HOSTCTRL1_SHDWN_OFFSET  2

#define BCM59055_REG_MBCCTRL3_WAC_HOSTEN     1 
#define BCM59055_REG_MBCCTRL3_USB_HOSTEN     2 
#define BCM59055_REG_MBCCTRL3_WAC_HAS_PRIORITY        1
#define BCM59055_REG_MBCCTRL3_WAC_HAS_PRIORITY_SHIFT  2

#define BCM59055_REG_MBCCTRL6_VFLOATMAX_4_2V  0x08
#define BCM59055_REG_MBCCTRL8_ICCMAX_940MA    0x07
#define BCM59055_REG_MBCCTRL9_SWUP_ON         0x01

#define BCM59055_REG_PWMLEDCTRL1 0x0

#define BCM59055_REG_PWMLEDCTRL1_HWSW_CNTRL_SW          0 

#define BCM59055_REG_PWMLEDCTRL1_PWMLED_PD_DISABLE        1 
#define BCM59055_REG_PWMLEDCTRL1_PWMLED_PD_DISABLE_SHIFT  1 

#define BCM59055_REG_PWMLEDCTRL1_BREATHING_REPEAT       1 
#define BCM59055_REG_PWMLEDCTRL1_BREATHING_REPEAT_SHIFT 2 

#define BCM59055_REG_PWMLEDCTRL1_PWM_FREQ_256           2 
#define BCM59055_REG_PWMLEDCTRL1_PWM_FREQ_256_SHIFT     5 

/* PMU Interrupts IDs  --------------------------------------------------------------- */
typedef enum {
	BCM59055_IRQID_INT1_POK_PRESSED,
	BCM59055_IRQID_INT1_POK_RELEASED,
	BCM59055_IRQID_INT1_POK_WAKEUP,	
	BCM59055_IRQID_INT1_POK_BIT_VLD,
	BCM59055_IRQID_INT1_POK_SHDWN,
	BCM59055_IRQID_INT1_AUX_INS,
	BCM59055_IRQID_INT1_AUX_RM,
	BCM59055_IRQID_INT1_GBAT_PLUG_IN,

	BCM59055_IRQID_INT2_CHGINS,
	BCM59055_IRQID_INT2_CHGRM,
	BCM59055_IRQID_INT2_CHGOV_DIS,
	BCM59055_IRQID_INT2_WAC_C_REDUCE,
	BCM59055_IRQID_INT2_USBINS,
	BCM59055_IRQID_INT2_USBRM,	
	BCM59055_IRQID_INT2_UBPD_CHP_DIS,
	BCM59055_IRQID_INT2_USB_CC_REDUCE,

	BCM59055_TOTAL_IRQ
} BCM59038_InterruptId;


struct bcm590xx_regulator_init_data ; 

void bcm59055_register_details(void **reg_info) ;
void bcm59055_regulator_desc(void **info) ;

void bcm59055_reg_init_dev_init(struct bcm590xx *bcm590xx)  ;

#endif

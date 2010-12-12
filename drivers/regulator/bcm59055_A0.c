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

#include <linux/regulator/driver.h>
#include <linux/mfd/bcm590xx/pmic.h>

/** Voltage in micro volts */
u32 bcm59055_ldo_v_table[] = 
{
	1300000, /* 0x000 */
	1800000, /* 0x001 */
	2500000, /* 0x010 */
	2700000, /* 0x011 */
	2800000, /* 0x100 */
	2900000, /* 0x101 */
	3000000, /* 0x110 */
	3300000, /* 0x111 */
} ;

struct regulator_desc bcm59055_info[NUM_BCM590XX_REGULATORS] = 
{
    { .name = "rfldo" , .id =   BCM59055_RFLDO , .n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "camldo" , .id =   BCM59055_CAMLDO , .n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "hv1ldo" , .id =   BCM59055_HV1LDO , .n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "hv2ldo" , .id =   BCM59055_HV2LDO , .n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "hv3ldo" , .id =   BCM59055_HV3LDO , .n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "hv4ldo" , .id =   BCM59055_HV4LDO , .n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "hv5ldo" , .id =   BCM59055_HV5LDO , .n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "hv6ldo" , .id =   BCM59055_HV6LDO , .n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "hv7ldo" , .id =   BCM59055_HV7LDO , .n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "simldo" , .id =   BCM59055_SIMLDO , .n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

} ;

struct bcm590xx_reg_info bcm59055_register_info[] =                       
{                                                                                                       
   {  
      .reg_addr =      BCM59055_REG_RFOPMODCTRL,     /* BCM59055_REGULATOR_RFLDO                           */
      .reg_addr_volt = BCM59055_REG_RFLDOCTRL,     /* BCM59035_REGULATOR_RFLDO                           */
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
   },  

   {  
      .reg_addr=      BCM59055_REG_CAMOPMODCTRL,     /* BCM59035_REGULATOR_CAMLDO                          */
      .reg_addr_volt= BCM59055_REG_CAMLDOCTRL,     /* BCM59035_REGULATOR_CAMLDO                           */
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
   },                                         

   {   
      .reg_addr=      BCM59055_REG_HV1OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO1CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
   },

   {   
      .reg_addr=      BCM59055_REG_HV2OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO2CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
   },

   {   
      .reg_addr=      BCM59055_REG_HV3OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO3CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
   },

   {   
      .reg_addr=      BCM59055_REG_HV4OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO4CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
   },

   {   
      .reg_addr=      BCM59055_REG_HV5OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO5CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
   },

   {   
      .reg_addr=      BCM59055_REG_HV6OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO6CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
   },

   {   
      .reg_addr=      BCM59055_REG_HV7OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO7CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
   },

   {   
      .reg_addr=      BCM59055_REG_SIMOPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_SIMLDOCTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
   },

} ;

void bcm59055_register_details(void **reg_info)
{
    *reg_info = &bcm59055_register_info[0] ;
}

void bcm59055_regulator_desc(void **info) 
{
    *info = &bcm59055_info[0] ;
}


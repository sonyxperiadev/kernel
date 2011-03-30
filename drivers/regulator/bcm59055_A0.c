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
#include <linux/mfd/bcm590xx/bcm59055_A0.h>

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

u32 bcm59055_sr_v_table[] = 
{
         700000,  /* 0x0000 */
	 800000,  /* 0x0001 */
	 860000,  /* 0x0010 */
	 880000,  /* 0x0011 */
	 900000,  /* 0x0100 */
	 920000,  /* 0x0101 */
	 940000,  /* 0x0110 */
	 960000,  /* 0x0111 */
	 980000,  /* 0x1000 */
	1000000,  /* 0x1001 */
	1020000,  /* 0x1010 */
	1040000,  /* 0x1011 */
	1060000,  
	1080000,  
	1100000, 
	1120000,  
	1140000, 
	1160000, 
	1180000, 
	1200000, 
	1220000, 
	1240000, 
	1260000, 
	1280000, 
	1300000, 
	1320000, 
	1340000, 
	1800000, 
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

    { .name = "csr" , .id =   BCM59055_CSR , .n_voltages = (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "iosr" , .id =   BCM59055_IOSR , .n_voltages = (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

    { .name = "sdsr" , .id =   BCM59055_SDSR , .n_voltages = (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
      .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
    },

} ;


struct bcm590xx_reg_info bcm59055_register_info[] =                       
{                                                                                                       
   {  
      .reg_addr =      BCM59055_REG_RFOPMODCTRL,     /* BCM59055_REGULATOR_RFLDO                           */
      .reg_addr_volt = BCM59055_REG_RFLDOCTRL,     /* BCM59035_REGULATOR_RFLDO                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
      .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	  .mode = LDO_STANDBY ,
   },  

   {  
      .reg_addr=      BCM59055_REG_CAMOPMODCTRL,     /* BCM59035_REGULATOR_CAMLDO                          */
      .reg_addr_volt= BCM59055_REG_CAMLDOCTRL,     /* BCM59035_REGULATOR_CAMLDO                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	  .mode = LDO_STANDBY ,
   },                                         

   {   
      .reg_addr=      BCM59055_REG_HV1OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO1CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	  .mode = LDO_STANDBY ,
   },

   {   
      .reg_addr=      BCM59055_REG_HV2OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO2CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	  .mode = LDO_STANDBY ,
   },

   {   
      .reg_addr=      BCM59055_REG_HV3OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO3CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	  .mode = LDO_STANDBY ,
   },

   {   
      .reg_addr=      BCM59055_REG_HV4OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO4CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	  .mode = LDO_STANDBY ,
   },

   {   
      .reg_addr=      BCM59055_REG_HV5OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO5CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	  .mode = LDO_STANDBY ,
   },

   {   
      .reg_addr=      BCM59055_REG_HV6OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO6CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	  .mode = LDO_STANDBY ,
   },

   {   
      .reg_addr=      BCM59055_REG_HV7OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_HVLDO7CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	  .mode = LDO_STANDBY ,
   },

   {   
      .reg_addr=      BCM59055_REG_SIMOPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
      .reg_addr_volt= BCM59055_REG_SIMLDOCTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x7,
      .vout_shift=    0,
      .v_table= bcm59055_ldo_v_table,
	  .num_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	  .mode = LDO_STANDBY ,
   },

   {  
      .reg_addr =      BCM59055_REG_CSROPMODCTRL,  /* BCM59055_REG_CSROPMODCTRL                          */
      .reg_addr_volt = BCM59055_REG_CSRCTRL1,      /* BCM59055_REG_CSRCTRL1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x1F,
      .vout_shift=    0,
      .v_table= bcm59055_sr_v_table,
	  .num_voltages = (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
	  .mode = LDO_STANDBY ,
   },  

   {  
      .reg_addr =      BCM59055_REG_IOSROPMODCTRL,  /* BCM59055_REG_IOSROPMODCTRL                          */
      .reg_addr_volt = BCM59055_REG_IOSRCTRL1,      /* BCM59055_REG_IOSRCTRL1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x1F,
      .vout_shift=    0,
      .v_table= bcm59055_sr_v_table,
	  .num_voltages = (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
	  .mode = LDO_STANDBY ,
   },  

   {  
      .reg_addr =      BCM59055_REG_SDSROPMODCTRL,  /* BCM59055_REG_SDSROPMODCTRL                          */
      .reg_addr_volt = BCM59055_REG_SDSRCTRL1,      /* BCM59055_REG_SDSRCTRL1                           */
	  .en_dis_mask   = 0x3 ,
	  .en_dis_shift  = EN_DIS_SHIFT ,
      .vout_mask=     0x1F,
      .vout_shift=    0,
      .v_table= bcm59055_sr_v_table,
	  .num_voltages = (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
	  .mode = LDO_STANDBY ,
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

/*****************************************************************************************/
/************************* I S L A N D                  S E T T I N G S ******************/
/*****************************************************************************************/
#define BCM590XX_REG_ENABLED  1
#define BCM590XX_REG_DISABLED 0

#define BCM59055_RFLDO_OTP_VAL     BCM590XX_REG_ENABLED 
#define BCM59055_CAMLDO_OTP_VAL    BCM590XX_REG_ENABLED 
#define BCM59055_HV1LDO_OTP_VAL    BCM590XX_REG_ENABLED 
#define BCM59055_HV2LDO_OTP_VAL    BCM590XX_REG_ENABLED 
#define BCM59055_HV3LDO_OTP_VAL    BCM590XX_REG_DISABLED 
#define BCM59055_HV4LDO_OTP_VAL    BCM590XX_REG_ENABLED 
#define BCM59055_HV5LDO_OTP_VAL    BCM590XX_REG_ENABLED 
#define BCM59055_HV6LDO_OTP_VAL    BCM590XX_REG_DISABLED 
#define BCM59055_HV7LDO_OTP_VAL    BCM590XX_REG_DISABLED 
#define BCM59055_SIMLDO_OTP_VAL    BCM590XX_REG_ENABLED 
#define BCM59055_CSR_OTP_VAL       BCM590XX_REG_ENABLED 
#define BCM59055_IOSR_OTP_VAL      BCM590XX_REG_ENABLED 
#define BCM59055_SDSR_OTP_VAL      BCM590XX_REG_ENABLED 

/*********** RFLDO Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply rf_supply =  { .supply = "rf_voltage" } ;

static struct regulator_init_data bcm59055_rfldo_data =  { 
.constraints = { .name = "rfldo", .min_uV = 1300000, .max_uV = 3300000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &rf_supply ,
};

static struct regulator_bulk_data bcm59055_bd_rf =                    { .supply = "rf_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_rf = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_rf, .init_on = BCM59055_RFLDO_OTP_VAL};
static struct platform_device bcm59055_uc_device_rf =                 { .name = "reg-userspace-consumer", .id = 2,           .dev = { .platform_data = &bcm59055_uc_data_rf, }, };
static struct platform_device bcm59055_vc_device_rf =                 { .name = "reg-virt-consumer",      .id = 2,           .dev = { .platform_data = "rf_voltage" , }, };
/*********** RFLDO END ********************************************************************************************************************************************************/

/*********** CAMLDO Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply cam_supply = { .supply = "cam_voltage" } ;

static struct regulator_init_data bcm59055_camldo_data = { 
.constraints = { .name = "camldo", .min_uV = 1300000, .max_uV = 3300000, .valid_ops_mask = REGULATOR_CHANGE_STATUS |REGULATOR_CHANGE_MODE |  REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &cam_supply ,
};

static struct regulator_bulk_data bcm59055_bd_cam =                    { .supply = "cam_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_cam = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_cam, .init_on = BCM59055_CAMLDO_OTP_VAL };
static struct platform_device bcm59055_uc_device_cam =                 { .name = "reg-userspace-consumer", .id = 3,           .dev = { .platform_data = &bcm59055_uc_data_cam, }, };
static struct platform_device bcm59055_vc_device_cam =                 { .name = "reg-virt-consumer",      .id = 3,           .dev = { .platform_data = "cam_voltage" , }, };
/*********** CAMLDO END ********************************************************************************************************************************************************/

/*********** HV1LDO Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply hv1_supply = { .supply = "hv1_voltage" } ;

static struct regulator_init_data bcm59055_hv1ldo_data = { 
.constraints = { .name = "hv1ldo", .min_uV = 1300000, .max_uV = 3300000, .valid_ops_mask = REGULATOR_CHANGE_STATUS |REGULATOR_CHANGE_MODE |  REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &hv1_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv1 =                    { .supply = "hv1_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv1 = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_hv1, .init_on = BCM59055_HV1LDO_OTP_VAL, };  
static struct platform_device bcm59055_uc_device_hv1 =                 { .name = "reg-userspace-consumer", .id = 4,           .dev = { .platform_data = &bcm59055_uc_data_hv1, }, };
static struct platform_device bcm59055_vc_device_hv1 =                 { .name = "reg-virt-consumer",      .id = 4,           .dev = { .platform_data = "hv1_voltage" , }, };
/*********** HV1LDO END ********************************************************************************************************************************************************/

/*********** HV2LDO Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply hv2_supply = { .supply = "hv2_voltage" } ;

static struct regulator_init_data bcm59055_hv2ldo_data = { 
.constraints = { .name = "hv2ldo", .min_uV = 1300000, .max_uV = 3300000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &hv2_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv2 =                    { .supply = "hv2_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv2 = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_hv2, .init_on = BCM59055_HV2LDO_OTP_VAL, };  
static struct platform_device bcm59055_uc_device_hv2 =                 { .name = "reg-userspace-consumer", .id = 5,           .dev = { .platform_data = &bcm59055_uc_data_hv2, }, };
static struct platform_device bcm59055_vc_device_hv2 =                 { .name = "reg-virt-consumer",      .id = 5,           .dev = { .platform_data = "hv2_voltage" , }, };
/*********** HV2LDO END ********************************************************************************************************************************************************/

/*********** HV3LDO Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply hv3_supply = { .supply = "hv3_voltage" } ;

static struct regulator_init_data bcm59055_hv3ldo_data = { 
.constraints = { .name = "hv3ldo", .min_uV = 1300000, .max_uV = 3300000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &hv3_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv3 =                    { .supply = "hv3_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv3 = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_hv3, .init_on = BCM59055_HV3LDO_OTP_VAL,};  
static struct platform_device bcm59055_uc_device_hv3 =                 { .name = "reg-userspace-consumer", .id = 6,           .dev = { .platform_data = &bcm59055_uc_data_hv3, }, };
static struct platform_device bcm59055_vc_device_hv3 =                 { .name = "reg-virt-consumer",      .id = 6,           .dev = { .platform_data = "hv3_voltage" , }, };
/*********** HV3LDO END ********************************************************************************************************************************************************/

/*********** HV4LDO Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply hv4_supply = { .supply = "hv4_voltage" } ;

static struct regulator_init_data bcm59055_hv4ldo_data = { 
.constraints = { .name = "hv4ldo", .min_uV = 1300000, .max_uV = 3300000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &hv4_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv4 =                    { .supply = "hv4_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv4 = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_hv4, .init_on = BCM59055_HV4LDO_OTP_VAL,};  
static struct platform_device bcm59055_uc_device_hv4 =                 { .name = "reg-userspace-consumer", .id = 7,           .dev = { .platform_data = &bcm59055_uc_data_hv4, }, };
static struct platform_device bcm59055_vc_device_hv4 =                 { .name = "reg-virt-consumer",      .id = 7,           .dev = { .platform_data = "hv4_voltage" , }, };
/*********** HV4LDO END ********************************************************************************************************************************************************/

/*********** HV5LDO Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply hv5_supply = { .supply = "hv5_voltage" } ;

static struct regulator_init_data bcm59055_hv5ldo_data = { 
.constraints = { .name = "hv5ldo", .min_uV = 1300000, .max_uV = 3300000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &hv5_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv5 =                    { .supply = "hv5_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv5 = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_hv5, .init_on = BCM59055_HV5LDO_OTP_VAL,};  
static struct platform_device bcm59055_uc_device_hv5 =                 { .name = "reg-userspace-consumer", .id = 8,           .dev = { .platform_data = &bcm59055_uc_data_hv5, }, };
static struct platform_device bcm59055_vc_device_hv5 =                 { .name = "reg-virt-consumer",      .id = 8,           .dev = { .platform_data = "hv5_voltage" , }, };
/*********** HV5LDO END ********************************************************************************************************************************************************/

/*********** HV6LDO Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply hv6_supply = { .supply = "hv6_voltage" } ;

static struct regulator_init_data bcm59055_hv6ldo_data = { 
.constraints = { .name = "hv6ldo", .min_uV = 1300000, .max_uV = 3300000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &hv6_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv6 =                    { .supply = "hv6_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv6 = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_hv6, .init_on = BCM59055_HV6LDO_OTP_VAL,};  
static struct platform_device bcm59055_uc_device_hv6 =                 { .name = "reg-userspace-consumer", .id = 9,           .dev = { .platform_data = &bcm59055_uc_data_hv6, }, };
static struct platform_device bcm59055_vc_device_hv6 =                 { .name = "reg-virt-consumer",      .id = 9,           .dev = { .platform_data = "hv6_voltage" , }, };
/*********** HV6LDO END ********************************************************************************************************************************************************/

/*********** HV7LDO Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply hv7_supply = { .supply = "hv7_voltage" } ;

static struct regulator_init_data bcm59055_hv7ldo_data = { 
.constraints = { .name = "hv7ldo", .min_uV = 1300000, .max_uV = 3300000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &hv7_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv7 =                    { .supply = "hv7_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv7 = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_hv7, .init_on = BCM59055_HV7LDO_OTP_VAL,};  
static struct platform_device bcm59055_uc_device_hv7 =                 { .name = "reg-userspace-consumer", .id = 10,           .dev = { .platform_data = &bcm59055_uc_data_hv7, }, };
static struct platform_device bcm59055_vc_device_hv7 =                 { .name = "reg-virt-consumer",      .id = 10,           .dev = { .platform_data = "hv7_voltage" , }, };
/*********** HV7LDO END ********************************************************************************************************************************************************/


/*********** SIMLDO Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply sim_supply = { .supply = "sim_voltage" } ;

static struct regulator_init_data bcm59055_simldo_data = { 
.constraints = { .name = "simldo", .min_uV = 1300000, .max_uV = 3300000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY}, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &sim_supply ,
};

static struct regulator_bulk_data bcm59055_bd_sim =                    { .supply = "sim_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_sim = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_sim, .init_on = BCM59055_SIMLDO_OTP_VAL,};  
static struct platform_device bcm59055_uc_device_sim =                 { .name = "reg-userspace-consumer", .id = 11,           .dev = { .platform_data = &bcm59055_uc_data_sim, }, };
static struct platform_device bcm59055_vc_device_sim =                 { .name = "reg-virt-consumer",      .id = 11,           .dev = { .platform_data = "sim_voltage" , }, };
/*********** SIMLDO END ********************************************************************************************************************************************************/

/*********** CSR Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply csr_supply = { .supply = "csr_voltage" } ;

static struct regulator_init_data bcm59055_csr_data = { 
.constraints = { .name = "csrldo", .min_uV = 700000, .max_uV = 1800000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY  }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &csr_supply ,
};

static struct regulator_bulk_data bcm59055_bd_csr =                    { .supply = "csr_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_csr = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_csr, .init_on = BCM59055_CSR_OTP_VAL,};  
static struct platform_device bcm59055_uc_device_csr =                 { .name = "reg-userspace-consumer", .id = 12,           .dev = { .platform_data = &bcm59055_uc_data_csr, }, };
static struct platform_device bcm59055_vc_device_csr =                 { .name = "reg-virt-consumer",      .id = 12,           .dev = { .platform_data = "csr_voltage" , }, };
/*********** CSR END ********************************************************************************************************************************************************/

/*********** IOSR Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply iosr_supply = { .supply = "iosr_voltage" } ;

static struct regulator_init_data bcm59055_iosr_data = { 
.constraints = { .name = "iosrldo", .min_uV = 700000, .max_uV = 1800000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &iosr_supply ,
};

static struct regulator_bulk_data bcm59055_bd_iosr =                    { .supply = "iosr_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_iosr = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_iosr, .init_on = BCM59055_IOSR_OTP_VAL,};  
static struct platform_device bcm59055_uc_device_iosr =                 { .name = "reg-userspace-consumer", .id = 13,           .dev = { .platform_data = &bcm59055_uc_data_iosr, }, };
static struct platform_device bcm59055_vc_device_iosr =                 { .name = "reg-virt-consumer",      .id = 13,           .dev = { .platform_data = "iosr_voltage" , }, };
/*********** IOSR END ********************************************************************************************************************************************************/

/*********** SDSR Start ******************************************************************************************************************************************************/
struct regulator_consumer_supply sdsr_supply = { .supply = "sdsr_voltage" } ;

static struct regulator_init_data bcm59055_sdsr_data = { 
.constraints = { .name = "sdsrldo", .min_uV = 700000, .max_uV = 1800000, .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , .always_on = 0, .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
.num_consumer_supplies = 1 ,
.consumer_supplies = &sdsr_supply ,
};

static struct regulator_bulk_data bcm59055_bd_sdsr =                    { .supply = "sdsr_voltage", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_sdsr = { .name = "bcm590xx",               .num_supplies = 1, .supplies = &bcm59055_bd_sdsr, .init_on = BCM59055_SDSR_OTP_VAL,};  
static struct platform_device bcm59055_uc_device_sdsr =                 { .name = "reg-userspace-consumer", .id = 14,           .dev = { .platform_data = &bcm59055_uc_data_sdsr, }, };
static struct platform_device bcm59055_vc_device_sdsr =                 { .name = "reg-virt-consumer",      .id = 14,           .dev = { .platform_data = "sdsr_voltage" , }, };
/*********** SDSR END ********************************************************************************************************************************************************/

struct bcm590xx_regulator_init_data bcm59055_regulators[] = 
{
	{ BCM59055_RFLDO, &bcm59055_rfldo_data },
	{ BCM59055_CAMLDO, &bcm59055_camldo_data },
	{ BCM59055_HV1LDO, &bcm59055_hv1ldo_data },
	{ BCM59055_HV2LDO, &bcm59055_hv2ldo_data },
	{ BCM59055_HV3LDO, &bcm59055_hv3ldo_data },
	{ BCM59055_HV4LDO, &bcm59055_hv4ldo_data },
	{ BCM59055_HV5LDO, &bcm59055_hv5ldo_data },
	{ BCM59055_HV6LDO, &bcm59055_hv6ldo_data },
	{ BCM59055_HV7LDO, &bcm59055_hv7ldo_data },
	{ BCM59055_SIMLDO, &bcm59055_simldo_data },
	{ BCM59055_CSR, &bcm59055_csr_data },
	{ BCM59055_IOSR, &bcm59055_iosr_data },
	{ BCM59055_SDSR, &bcm59055_sdsr_data }
};

struct platform_device *bcm59055_state_change_and_voltage_change_devices[] = {
    &bcm59055_uc_device_rf , &bcm59055_vc_device_rf ,
    &bcm59055_uc_device_cam , &bcm59055_vc_device_cam ,
    &bcm59055_uc_device_hv1 , &bcm59055_vc_device_hv1 ,
    &bcm59055_uc_device_hv2 , &bcm59055_vc_device_hv2 ,
    &bcm59055_uc_device_hv3 , &bcm59055_vc_device_hv3 ,
    &bcm59055_uc_device_hv4 , &bcm59055_vc_device_hv4 ,
    &bcm59055_uc_device_hv5 , &bcm59055_vc_device_hv5 ,
    &bcm59055_uc_device_hv6 , &bcm59055_vc_device_hv6 ,
    &bcm59055_uc_device_hv7 , &bcm59055_vc_device_hv7 ,
    &bcm59055_uc_device_sim , &bcm59055_vc_device_sim ,
    &bcm59055_uc_device_csr , &bcm59055_vc_device_csr ,
    &bcm59055_uc_device_iosr , &bcm59055_vc_device_iosr ,
    &bcm59055_uc_device_sdsr , &bcm59055_vc_device_sdsr ,
};

void bcm59055_reg_init_dev_init(struct bcm590xx *bcm590xx) 
{
    int i = 0 ;
    for (i = 0; i < ARRAY_SIZE(bcm59055_regulators); i++)
    {
        bcm590xx_register_regulator(bcm590xx,
                                    bcm59055_regulators[i].regulator,
                                    bcm59055_regulators[i].initdata);
    }

    platform_add_devices(bcm59055_state_change_and_voltage_change_devices, ARRAY_SIZE(bcm59055_state_change_and_voltage_change_devices));
}


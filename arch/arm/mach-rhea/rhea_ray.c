/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <asm/gpio.h>
#ifdef CONFIG_GPIO_PCA953X
#include <linux/i2c/pca953x.h>
#endif
#ifdef CONFIG_TOUCHSCREEN_QT602240
#include <linux/i2c/qt602240_ts.h>
#endif
#include <mach/kona_headset_pd.h>
#include <mach/kona.h>
#include <mach/rhea.h>
#include <asm/mach/map.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/clk.h>
#include "common.h"
#ifdef CONFIG_KEYBOARD_BCM
#include <mach/bcm_keypad.h>
#endif
#ifdef CONFIG_DMAC_PL330
#include <mach/irqs.h>
#include <plat/pl330-pdata.h>
#include <linux/dma-mapping.h>
#endif

#define PMU_DEVICE_I2C_ADDR_0   0x08
#define PMU_DEVICE_I2C_ADDR_1   0x0C
#define PMU_IRQ_PIN           10

// keypad map
#define BCM_KEY_ROW_0  0
#define BCM_KEY_ROW_1  1
#define BCM_KEY_ROW_2  2
#define BCM_KEY_ROW_3  3
#define BCM_KEY_ROW_4  4
#define BCM_KEY_ROW_5  5
#define BCM_KEY_ROW_6  6
#define BCM_KEY_ROW_7  7

#define BCM_KEY_COL_0  0
#define BCM_KEY_COL_1  1
#define BCM_KEY_COL_2  2
#define BCM_KEY_COL_3  3
#define BCM_KEY_COL_4  4
#define BCM_KEY_COL_5  5
#define BCM_KEY_COL_6  6
#define BCM_KEY_COL_7  7

#define PMU_DEVICE_I2C_ADDR_0   0x08 
#define PMU_DEVICE_I2C_ADDR_1   0x0C
#define PMU_IRQ_PIN           10

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

/*********** RFLDO ***********/
struct regulator_consumer_supply rf_supply =  { .supply = "rfldo_consumer_supply" } ;
static struct regulator_init_data bcm59055_rfldo_data =  { 
    .constraints = { 
        .name = "rfldo", 
        .min_uV = 1300000, 
        .max_uV = 3300000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY 
    }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &rf_supply ,
};
static struct regulator_bulk_data bcm59055_bd_rf = { 
    .supply = "rfldo_consumer_supply", 
};
static struct regulator_userspace_consumer_data bcm59055_uc_data_rf = { 
	.name = "bcm590xx",               
	.num_supplies = 1, 
	.supplies = &bcm59055_bd_rf, 
	.init_on = BCM59055_RFLDO_OTP_VAL
};
static struct platform_device bcm59055_uc_device_rf = { 
	.name = "reg-userspace-consumer", 
	.id = 0,           
	.dev = { .platform_data = &bcm59055_uc_data_rf, }, 
};
static struct platform_device bcm59055_vc_device_rf = { 
	.name = "reg-virt-consumer",
	.id = 0, 
	.dev = { .platform_data = "rfldo_consumer_supply" , }, 
};

/*********** CAMLDO **************/
struct regulator_consumer_supply cam_supply = { .supply = "camldo_consumer_supply" } ;

static struct regulator_init_data bcm59055_camldo_data = { 
    .constraints = { 
        .name = "camldo", 
        .min_uV = 1300000, 
        .max_uV = 3300000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS |REGULATOR_CHANGE_MODE |  REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY 
    }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &cam_supply ,
};

static struct regulator_bulk_data bcm59055_bd_cam = { .supply = "camldo_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_cam = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_cam, 
    .init_on = BCM59055_CAMLDO_OTP_VAL 
};
static struct platform_device bcm59055_uc_device_cam = { 
    .name = "reg-userspace-consumer", 
    .id = 1,           
    .dev = { .platform_data = &bcm59055_uc_data_cam, }, 
};
static struct platform_device bcm59055_vc_device_cam = { 
    .name = "reg-virt-consumer",      
    .id = 1,           
    .dev = { .platform_data = "camldo_consumer_supply" , }, 
};

/*********** HV1LDO ***************/
struct regulator_consumer_supply hv1_supply = { .supply = "hv1ldo_consumer_supply" } ;

static struct regulator_init_data bcm59055_hv1ldo_data = { 
    .constraints = { 
        .name = "hv1ldo", 
        .min_uV = 1300000, 
        .max_uV = 3300000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS |REGULATOR_CHANGE_MODE |  REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY 
    }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &hv1_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv1 = { .supply = "hv1ldo_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv1 = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_hv1, 
    .init_on = BCM59055_HV1LDO_OTP_VAL, 
};  
static struct platform_device bcm59055_uc_device_hv1 = { 
    .name = "reg-userspace-consumer", 
    .id = 2,           
    .dev = { .platform_data = &bcm59055_uc_data_hv1, }, 
};

static struct platform_device bcm59055_vc_device_hv1 = { 
    .name = "reg-virt-consumer",      
    .id = 2,           
    .dev = { .platform_data = "hv1ldo_consumer_supply" , }, 
};

/*********** HV2LDO **************/
struct regulator_consumer_supply hv2_supply = { .supply = "hv2ldo_consumer_supply" } ;

static struct regulator_init_data bcm59055_hv2ldo_data = { 
    .constraints = { 
        .name = "hv2ldo", 
        .min_uV = 1300000, 
        .max_uV = 3300000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY 
    }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &hv2_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv2 = { .supply = "hv2ldo_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv2 = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_hv2, 
    .init_on = BCM59055_HV2LDO_OTP_VAL, 
};  
static struct platform_device bcm59055_uc_device_hv2 = { 
    .name = "reg-userspace-consumer", 
    .id = 3,           
    .dev = { .platform_data = &bcm59055_uc_data_hv2, }, 
};
static struct platform_device bcm59055_vc_device_hv2 = { 
    .name = "reg-virt-consumer",      
    .id = 3,           
    .dev = { .platform_data = "hv2ldo_consumer_supply" , }, 
};

/*********** HV3LDO ********************/
struct regulator_consumer_supply hv3_supply = { .supply = "hv3ldo_consumer_supply" } ;

static struct regulator_init_data bcm59055_hv3ldo_data = { 
    .constraints = { 
        .name = "hv3ldo", 
        .min_uV = 1300000, 
        .max_uV = 3300000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &hv3_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv3 = { .supply = "hv3ldo_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv3 = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_hv3, 
    .init_on = BCM59055_HV3LDO_OTP_VAL,
};  
static struct platform_device bcm59055_uc_device_hv3 = { 
    .name = "reg-userspace-consumer", 
    .id = 4,           
    .dev = { .platform_data = &bcm59055_uc_data_hv3, }, 
};
static struct platform_device bcm59055_vc_device_hv3 = { 
    .name = "reg-virt-consumer",      
    .id = 4,           
    .dev = { .platform_data = "hv3ldo_consumer_supply" , }, 
};

/*********** HV4LDO ***************/
struct regulator_consumer_supply hv4_supply = { .supply = "hv4ldo_consumer_supply" } ;

static struct regulator_init_data bcm59055_hv4ldo_data = { 
    .constraints = { 
        .name = "hv4ldo", 
        .min_uV = 1300000, 
        .max_uV = 3300000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &hv4_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv4 =                    { .supply = "hv4ldo_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv4 = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_hv4, 
    .init_on = BCM59055_HV4LDO_OTP_VAL,
};  
static struct platform_device bcm59055_uc_device_hv4 = { 
    .name = "reg-userspace-consumer", 
    .id = 5,           
    .dev = { .platform_data = &bcm59055_uc_data_hv4, }, 
};
static struct platform_device bcm59055_vc_device_hv4 = { 
    .name = "reg-virt-consumer",      
    .id = 5,           
    .dev = { .platform_data = "hv4ldo_consumer_supply" , }, 
};

/*********** HV5LDO ****************/
struct regulator_consumer_supply hv5_supply = { .supply = "hv5ldo_consumer_supply" } ;

static struct regulator_init_data bcm59055_hv5ldo_data = { 
    .constraints = { 
        .name = "hv5ldo", 
        .min_uV = 1300000, 
        .max_uV = 3300000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &hv5_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv5 =                    { .supply = "hv5ldo_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv5 = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_hv5, 
    .init_on = BCM59055_HV5LDO_OTP_VAL,
};  
static struct platform_device bcm59055_uc_device_hv5 = { 
    .name = "reg-userspace-consumer", 
    .id = 6,           
    .dev = { .platform_data = &bcm59055_uc_data_hv5, }, 
};
static struct platform_device bcm59055_vc_device_hv5 = { 
    .name = "reg-virt-consumer",      
    .id = 6,           
    .dev = { .platform_data = "hv5ldo_consumer_supply" , }, 
};

/*********** HV6LDO *******************/
struct regulator_consumer_supply hv6_supply = { .supply = "hv6ldo_consumer_supply" } ;

static struct regulator_init_data bcm59055_hv6ldo_data = { 
    .constraints = { 
        .name = "hv6ldo", 
        .min_uV = 1300000, 
        .max_uV = 3300000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &hv6_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv6 =                    { .supply = "hv6ldo_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv6 = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_hv6, 
    .init_on = BCM59055_HV6LDO_OTP_VAL,
};  
static struct platform_device bcm59055_uc_device_hv6 =                 { 
    .name = "reg-userspace-consumer", 
    .id = 7,           
    .dev = { .platform_data = &bcm59055_uc_data_hv6, }, 
};
static struct platform_device bcm59055_vc_device_hv6 =                 { 
    .name = "reg-virt-consumer",      
    .id = 7,           
    .dev = { .platform_data = "hv6ldo_consumer_supply" , }, 
};

/*********** HV7LDO **************/
struct regulator_consumer_supply hv7_supply = { .supply = "hv7ldo_consumer_supply" } ;

static struct regulator_init_data bcm59055_hv7ldo_data = { 
    .constraints = { 
        .name = "hv7ldo", 
        .min_uV = 1300000, 
        .max_uV = 3300000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &hv7_supply ,
};

static struct regulator_bulk_data bcm59055_bd_hv7 =                    { .supply = "hv7ldo_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_hv7 = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_hv7, 
    .init_on = BCM59055_HV7LDO_OTP_VAL,
};  
static struct platform_device bcm59055_uc_device_hv7 = { 
    .name = "reg-userspace-consumer", 
    .id = 8,           
    .dev = { .platform_data = &bcm59055_uc_data_hv7, }, 
};
static struct platform_device bcm59055_vc_device_hv7 = { 
    .name = "reg-virt-consumer",      
    .id = 8,           
    .dev = { .platform_data = "hv7ldo_consumer_supply" , }, 
};

/*********** SIMLDO **************/
struct regulator_consumer_supply sim_supply = { .supply = "simldo_consumer_supply" } ;

static struct regulator_init_data bcm59055_simldo_data = { 
    .constraints = { 
        .name = "simldo", 
        .min_uV = 1300000, 
        .max_uV = 3300000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY}, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &sim_supply ,
};

static struct regulator_bulk_data bcm59055_bd_sim =                    { .supply = "simldo_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_sim = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_sim, 
    .init_on = BCM59055_SIMLDO_OTP_VAL,
};  
static struct platform_device bcm59055_uc_device_sim = { 
    .name = "reg-userspace-consumer", 
    .id = 9,           
    .dev = { .platform_data = &bcm59055_uc_data_sim, }, 
};
static struct platform_device bcm59055_vc_device_sim = { 
    .name = "reg-virt-consumer",      
    .id = 9,           
    .dev = { .platform_data = "simldo_consumer_supply" , }, 
};

/*********** CSR *************/
struct regulator_consumer_supply csr_supply = { .supply = "csr_consumer_supply" } ;

static struct regulator_init_data bcm59055_csr_data = { 
    .constraints = { 
        .name = "csrldo", 
        .min_uV = 700000, 
        .max_uV = 1800000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY  }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &csr_supply ,
};

static struct regulator_bulk_data bcm59055_bd_csr = { .supply = "csr_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_csr = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_csr, 
    .init_on = BCM59055_CSR_OTP_VAL,
};  

static struct platform_device bcm59055_uc_device_csr = { 
    .name = "reg-userspace-consumer", 
    .id = 10,           
    .dev = { .platform_data = &bcm59055_uc_data_csr, }, 
};

static struct platform_device bcm59055_vc_device_csr = { 
    .name = "reg-virt-consumer",      
    .id = 10,           
    .dev = { .platform_data = "csr_consumer_supply" , }, 
};

/*********** IOSR *************/
struct regulator_consumer_supply iosr_supply = { .supply = "iosr_consumer_supply" } ;

static struct regulator_init_data bcm59055_iosr_data = { 
    .constraints = { 
        .name = "iosrldo", 
        .min_uV = 700000, 
        .max_uV = 1800000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &iosr_supply ,
};

static struct regulator_bulk_data bcm59055_bd_iosr = { .supply = "iosr_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_iosr = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_iosr, 
    .init_on = BCM59055_IOSR_OTP_VAL,
};  
static struct platform_device bcm59055_uc_device_iosr = { 
    .name = "reg-userspace-consumer", 
    .id = 11,           
    .dev = { .platform_data = &bcm59055_uc_data_iosr, }, 
};
static struct platform_device bcm59055_vc_device_iosr = { 
    .name = "reg-virt-consumer",      
    .id = 11,           
    .dev = { .platform_data = "iosr_consumer_supply" , }, 
};

/*********** SDSR ***********/
struct regulator_consumer_supply sdsr_supply = { .supply = "sdsr_consumer_supply" } ;

static struct regulator_init_data bcm59055_sdsr_data = { 
    .constraints = { 
        .name = "sdsrldo", 
        .min_uV = 700000, 
        .max_uV = 1800000, 
        .valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE , 
        .always_on = 0, 
        .valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY }, 
    .num_consumer_supplies = 1 ,
    .consumer_supplies = &sdsr_supply ,
};

static struct regulator_bulk_data bcm59055_bd_sdsr =                    { .supply = "sdsr_consumer_supply", };
static struct regulator_userspace_consumer_data bcm59055_uc_data_sdsr = { 
    .name = "bcm590xx",               
    .num_supplies = 1, 
    .supplies = &bcm59055_bd_sdsr, 
    .init_on = BCM59055_SDSR_OTP_VAL,
};  
static struct platform_device bcm59055_uc_device_sdsr = { 
    .name = "reg-userspace-consumer", 
    .id = 12,           
    .dev = { .platform_data = &bcm59055_uc_data_sdsr, }, 
};
static struct platform_device bcm59055_vc_device_sdsr = { 
    .name = "reg-virt-consumer",      
    .id = 12,           
    .dev = { .platform_data = "sdsr_consumer_supply" , }, 
};

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

static int __init bcm590xx_init_platform_hw(struct bcm590xx *bcm590xx)
{
    int i = 0 ;
    printk("REG: pmu_init_platform_hw called \n") ;

    for (i = 0; i < ARRAY_SIZE(bcm59055_regulators); i++)
    {
        bcm590xx_register_regulator(bcm590xx,
                                    bcm59055_regulators[i].regulator,
                                    bcm59055_regulators[i].initdata);
    }

    platform_add_devices(bcm59055_state_change_and_voltage_change_devices, ARRAY_SIZE(bcm59055_state_change_and_voltage_change_devices));
    return 0 ;
}

#ifdef CONFIG_BATTERY_BCM59055
/* wall charging and vbus are wired together on FF board
     we monitor USB activity to make sure it is not USB cable that is inserted
 */
static int can_start_charging(void* data)
{
#define INTERVAL (HZ/10)
	int cpu, usb_otg_int[4], i;
	for_each_present_cpu(cpu)
		usb_otg_int[cpu] =  kstat_irqs_cpu(
		BCM_INT_ID_USB_HSOTG, cpu);

	for (i=0; i<10; i++) {
		schedule_timeout_interruptible(INTERVAL);
		for_each_present_cpu(cpu)
			if (usb_otg_int[cpu]!= kstat_irqs_cpu(
				BCM_INT_ID_USB_HSOTG, cpu))
				return 0;
	}
	return 1;
}

static struct mv_percent mv_percent_table[] = 
{
    { 3800 , 5 },
    { 3850 , 25 },
    { 3900 , 50 },
    { 3950 , 70 },
    { 4000 , 90 },
    { 4100 , 100 },
} ;

static struct bcm590xx_battery_pdata bcm590xx_battery_plat_data = {
	.can_start_charging = can_start_charging,
	.vp_table = mv_percent_table ,
	.vp_table_cnt = ARRAY_SIZE(mv_percent_table) ,
        .batt_min_volt = 3200 ,
        .batt_max_volt = 4200 ,
        .batt_technology = POWER_SUPPLY_TECHNOLOGY_LION ,
};
#endif

static struct bcm590xx_platform_data bcm590xx_plat_data = {
	.init = bcm590xx_init_platform_hw,
	.slave = 0 ,
#ifdef CONFIG_BATTERY_BCM59055
	.battery_pdata = &bcm590xx_battery_plat_data,
#endif
};

static struct bcm590xx_platform_data bcm590xx_plat_data_sl1 = {
	.slave = 1 ,
};

static struct i2c_board_info __initdata pmu_info[] = 
{
   {  
      I2C_BOARD_INFO("bcm590xx", PMU_DEVICE_I2C_ADDR_1 ), 
      .irq = gpio_to_irq(PMU_IRQ_PIN),
      .platform_data  = &bcm590xx_plat_data_sl1,
   },
   {  
      I2C_BOARD_INFO("bcm590xx", PMU_DEVICE_I2C_ADDR_0 ), 
      .irq = gpio_to_irq(PMU_IRQ_PIN),
      .platform_data  = &bcm590xx_plat_data, 
   },
};

#ifdef CONFIG_KEYBOARD_BCM
/*!
 * The keyboard definition structure.
 */
struct platform_device bcm_kp_device = {
	.name = "bcm_keypad",
	.id = -1,
};

/*	Keymap for Ray board plug-in 64-key keypad. 
	Since LCD block has used pin GPIO00, GPIO01, GPIO02, GPIO03,
	GPIO08, GPIO09, GPIO10 and GPIO11, Keypad can be set as 4x4 matric by
	using pin GPIO04, GPIO05, GPIO06, GPIO07, GPIO12, GPIO13, GPIO14 and
	GPIO15 */
static struct bcm_keymap newKeymap[] = {
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_4, "Search Key", KEY_SEARCH},
	{BCM_KEY_ROW_4, BCM_KEY_COL_5, "Back Key", KEY_BACK},
	{BCM_KEY_ROW_4, BCM_KEY_COL_6, "Forward key", KEY_FORWARD},
	{BCM_KEY_ROW_4, BCM_KEY_COL_7, "Home Key", KEY_HOME},
	{BCM_KEY_ROW_5, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_4, "Menu-Key", KEY_MENU},
	{BCM_KEY_ROW_5, BCM_KEY_COL_5, "VolumnUp-Key", KEY_VOLUMEUP},
	{BCM_KEY_ROW_5, BCM_KEY_COL_6, "VolumnDown-Key", KEY_VOLUMEDOWN},
	{BCM_KEY_ROW_5, BCM_KEY_COL_7, "key mute", KEY_MUTE},
	{BCM_KEY_ROW_6, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_4, "key space", KEY_SPACE},
	{BCM_KEY_ROW_6, BCM_KEY_COL_5, "key power", KEY_POWER},
	{BCM_KEY_ROW_6, BCM_KEY_COL_6, "key sleep", KEY_SLEEP},
	{BCM_KEY_ROW_6, BCM_KEY_COL_7, "key wakeup", KEY_WAKEUP},
	{BCM_KEY_ROW_7, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_7, "unused", 0},
};

static struct bcm_keypad_platform_info bcm_keypad_data = {
	.row_num = 8,
	.col_num = 8,
	.keymap = newKeymap,
	.bcm_keypad_base = (void *)__iomem HW_IO_PHYS_TO_VIRT(KEYPAD_BASE_ADDR),
};

#endif

#ifdef CONFIG_GPIO_PCA953X
#define GPIO_PCA953X_GPIO_PIN      74 /* Configure pad MMC1DAT4 to GPIO74 */
static int pca953x_platform_init_hw(struct i2c_client *client,
		unsigned gpio, unsigned ngpio, void *context)
{
	int rc;
	rc = gpio_request(GPIO_PCA953X_GPIO_PIN, "gpio_expander");
	if (rc < 0)
	{
		printk(KERN_ERR "unable to request GPIO pin %d\n", GPIO_PCA953X_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(GPIO_PCA953X_GPIO_PIN);
	return 0;
}

static int pca953x_platform_exit_hw(struct i2c_client *client,
		unsigned gpio, unsigned ngpio, void *context)
{
	gpio_free(GPIO_PCA953X_GPIO_PIN);
	return 0;
}

static struct pca953x_platform_data board_expander_info = {
	.gpio_base	= KONA_MAX_GPIO,
	.irq_base	= gpio_to_irq(KONA_MAX_GPIO),
	.setup		= pca953x_platform_init_hw,
	.teardown	= pca953x_platform_exit_hw,
};

static struct i2c_board_info __initdata pca953x_info[] = {
	{
		I2C_BOARD_INFO("pca9539", 0x74),
		.irq = gpio_to_irq(GPIO_PCA953X_GPIO_PIN),
		.platform_data = &board_expander_info,
	},
};
#endif /* CONFIG_GPIO_PCA953X */

#ifdef CONFIG_TOUCHSCREEN_QT602240
#ifdef CONFIG_GPIO_PCA953X
#define QT602240_INT_GPIO_PIN      (KONA_MAX_GPIO + 8)
#else
#define QT602240_INT_GPIO_PIN      74 /* skip expander chip */
#endif
static int qt602240_platform_init_hw(void)
{
	int rc;
	rc = gpio_request(QT602240_INT_GPIO_PIN, "ts_qt602240");
	if (rc < 0)
	{
		printk(KERN_ERR "unable to request GPIO pin %d\n", QT602240_INT_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(QT602240_INT_GPIO_PIN);

	return 0;
}

static void qt602240_platform_exit_hw(void)
{
	gpio_free(QT602240_INT_GPIO_PIN);
}

static struct qt602240_platform_data qt602240_platform_data = {
	.x_line		= 17,
	.y_line		= 11,
	.x_size		= 800,
	.y_size		= 480,
	.blen		= 0x21,
	.threshold	= 0x28,
	.voltage	= 2800000,              /* 2.8V */
	.orient		= QT602240_DIAGONAL_COUNTER,
	.init_platform_hw = qt602240_platform_init_hw,
	.exit_platform_hw = qt602240_platform_exit_hw,
};

static struct i2c_board_info __initdata qt602240_info[] = {
	{
		I2C_BOARD_INFO("qt602240_ts", 0x4a),
		.platform_data = &qt602240_platform_data,
		.irq = gpio_to_irq(QT602240_INT_GPIO_PIN),
	},
};
#endif /* CONFIG_TOUCHSCREEN_QT602240 */

#ifdef CONFIG_KONA_HEADSET
#define HS_IRQ	gpio_to_irq(71)
#define HSB_IRQ	BCM_INT_ID_AUXMIC_COMP1
static struct kona_headset_pd headset_data = {
	.hs_default_state = 1, /* GPIO state read is 0 on HS insert and 1 for
							* HS remove*/
};

static struct resource board_headset_resource[] = {
	{	/* For AUXMIC */
		.start = AUXMIC_BASE_ADDR,
		.end = AUXMIC_BASE_ADDR + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{	/* For ACI */
		.start = ACI_BASE_ADDR,
		.end = ACI_BASE_ADDR + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{	/* For Headset IRQ */
		.start = HS_IRQ,
		.end = HS_IRQ,
		.flags = IORESOURCE_IRQ,
	},
	{	/* For Headset button IRQ */
		.start = HSB_IRQ,
		.end = HSB_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device headset_device = {
	.name = "konaheadset",
	.id = -1,
	.resource = board_headset_resource,
	.num_resources	= ARRAY_SIZE(board_headset_resource),
	.dev	=	{
		.platform_data = &headset_data,
	},
};
#endif /* CONFIG_KONA_HEADSET */

#ifdef CONFIG_DMAC_PL330
static struct kona_pl330_data rhea_pl330_pdata =	{
	/* Non Secure DMAC virtual base address */
	.dmac_ns_base = KONA_DMAC_NS_VA,
	/* Secure DMAC virtual base address */
	.dmac_s_base = KONA_DMAC_S_VA,
	/* # of PL330 dmac channels 'configurable' */
	.num_pl330_chans = 8,
	/* irq number to use */
	.irq_base = BCM_INT_ID_RESERVED184,
	/* # of PL330 Interrupt lines connected to GIC */
	.irq_line_count = 8,
};

static struct platform_device pl330_dmac_device = {
	.name = "kona-dmac-pl330",
	.id = 0,
	.dev = {
		.platform_data = &rhea_pl330_pdata,
		.coherent_dma_mask  = DMA_BIT_MASK(64),
	},
};
#endif

/* Rhea Ray specific platform devices */ 
static struct platform_device *rhea_ray_plat_devices[] __initdata = {
#ifdef CONFIG_KEYBOARD_BCM
	&bcm_kp_device,
#endif

#ifdef CONFIG_KONA_HEADSET
	&headset_device,
#endif

#ifdef CONFIG_DMAC_PL330
	&pl330_dmac_device,
#endif
};

/* Rhea Ray specific i2c devices */ 
static void __init rhea_ray_add_i2c_devices (void)
{
	/* 59055 on BSC - PMU */
	i2c_register_board_info(2,
		pmu_info,
		ARRAY_SIZE(pmu_info));
}

static int __init rhea_ray_add_lateInit_devices (void)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = i2c_get_adapter(1);
	if (!adapter) {
		printk(KERN_ERR "can't get i2c adapter 1 %d\n");
		return ENODEV;
	}
#ifdef CONFIG_GPIO_PCA953X
	client = i2c_new_device(adapter, pca953x_info);
	if (!client) {
		printk(KERN_ERR "an't add i2c device for pca953x\n");
	}
#endif

#ifdef CONFIG_TOUCHSCREEN_QT602240
	client = i2c_new_device(adapter, qt602240_info);
	if (!client) {
		printk(KERN_ERR "an't add i2c device for qt602240\n");
	}
#endif
	i2c_put_adapter(adapter);

	board_add_sdio_devices();
	return 0;
}

static void enable_smi_display_clks(void)
{
	struct clk *smi_axi;
	struct clk *mm_dma;
	struct clk *smi;

	smi_axi = clk_get (NULL, "smi_axi_clk");
	mm_dma = clk_get (NULL, "mm_dma_axi_clk");

	smi = clk_get (NULL, "smi_clk");
	BUG_ON (!smi_axi || !smi || !mm_dma);


	clk_set_rate (smi, 250000000);

	clk_enable (smi_axi);
	clk_enable (smi);
	clk_enable(mm_dma);
}

/* All Rhea Ray specific devices */ 
static void __init rhea_ray_add_devices(void)
{
	enable_smi_display_clks();

#ifdef CONFIG_KEYBOARD_BCM
	bcm_kp_device.dev.platform_data = &bcm_keypad_data;
#endif
	platform_add_devices(rhea_ray_plat_devices, ARRAY_SIZE(rhea_ray_plat_devices));

	rhea_ray_add_i2c_devices();
}

void __init board_init(void)
{
	board_add_common_devices();
	rhea_ray_add_devices();
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	rhea_map_io();
}

late_initcall(rhea_ray_add_lateInit_devices);

MACHINE_START(RHEA, "RheaRay")
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.timer  = &kona_timer,
	.init_machine = board_init,
MACHINE_END

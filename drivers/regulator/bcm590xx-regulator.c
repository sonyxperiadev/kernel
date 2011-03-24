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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>

/** Voltage in micro volts */
static const u32 bcm59055_ldo_uv_table[] = 
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

static const u32 bcm59055_sr_uv_table[] = 
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

#define N_LDO_VOLTAGES ARRAY_SIZE(bcm59055_ldo_uv_table)
#define N_SR_VOLTAGES  ARRAY_SIZE(bcm59055_sr_uv_table) 

static struct regulator_desc bcm59055_info[NUM_BCM590XX_REGULATORS] = 
{
    {.name="rfldo",     .id=BCM59055_RFLDO, .n_voltages=N_LDO_VOLTAGES, .ops=&bcm590xxldo_ops, .type=REGULATOR_VOLTAGE, .owner=THIS_MODULE , },
    {.name="camldo",    .id=BCM59055_CAMLDO, .n_voltages=N_LDO_VOLTAGES, .ops=&bcm590xxldo_ops, .type=REGULATOR_VOLTAGE, .owner=THIS_MODULE,},
    {.name="hv1ldo" ,   .id=BCM59055_HV1LDO, .n_voltages=N_LDO_VOLTAGES, .ops=&bcm590xxldo_ops , .type=REGULATOR_VOLTAGE , .owner=THIS_MODULE , },
    {.name = "hv2ldo" , .id=BCM59055_HV2LDO , .n_voltages = N_LDO_VOLTAGES,.ops=&bcm590xxldo_ops , .type=REGULATOR_VOLTAGE, .owner=THIS_MODULE , },
    {.name = "hv3ldo" , .id=BCM59055_HV3LDO , .n_voltages = N_LDO_VOLTAGES, .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE , },
    {.name = "hv4ldo" , .id=BCM59055_HV4LDO , .n_voltages = N_LDO_VOLTAGES, .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE , },
    {.name = "hv5ldo" , .id=BCM59055_HV5LDO , .n_voltages = N_LDO_VOLTAGES, .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE , },
    {.name = "hv6ldo" , .id=BCM59055_HV6LDO , .n_voltages = N_LDO_VOLTAGES, .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE , },
    {.name = "hv7ldo" , .id=BCM59055_HV7LDO , .n_voltages = N_LDO_VOLTAGES, .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE , },
    {.name = "simldo" , .id=BCM59055_SIMLDO , .n_voltages = N_LDO_VOLTAGES, .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE , },
    {.name = "csr" ,    .id=BCM59055_CSR ,    .n_voltages = N_SR_VOLTAGES, .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE , },
    {.name = "iosr" ,   .id=BCM59055_IOSR ,   .n_voltages = N_SR_VOLTAGES , .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE , },
    {.name = "sdsr" ,   .id=BCM59055_SDSR ,   .n_voltages = N_SR_VOLTAGES , .ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE , },
} ;

#define EN_DIS_MASK    0x03
#define VOUT_LDO_MASK  0x07 
#define VOUT_SR_MASK   0x1F 

static struct bcm590xx_reg_info bcm59055_register_info[] =                       
{                                                                                                       
    {  
        .reg_addr =      BCM59055_REG_RFOPMODCTRL,     /* BCM59055_REGULATOR_RFLDO                           */
        .reg_addr_volt = BCM59055_REG_RFLDOCTRL,     /* BCM59035_REGULATOR_RFLDO                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_LDO_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_ldo_uv_table,
        .num_voltages = N_LDO_VOLTAGES ,
        .mode = LDO_STANDBY ,
    },  

    {  
        .reg_addr=      BCM59055_REG_CAMOPMODCTRL,     /* BCM59035_REGULATOR_CAMLDO                          */
        .reg_addr_volt= BCM59055_REG_CAMLDOCTRL,     /* BCM59035_REGULATOR_CAMLDO                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_LDO_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_ldo_uv_table,
        .num_voltages = N_LDO_VOLTAGES ,
        .mode = LDO_STANDBY ,
    },                                         

    {   
        .reg_addr=      BCM59055_REG_HV1OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
        .reg_addr_volt= BCM59055_REG_HVLDO1CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_LDO_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_ldo_uv_table,
        .num_voltages = N_LDO_VOLTAGES ,
        .mode = LDO_STANDBY ,
    },

    {   
        .reg_addr=      BCM59055_REG_HV2OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
        .reg_addr_volt= BCM59055_REG_HVLDO2CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_LDO_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_ldo_uv_table,
        .num_voltages = N_LDO_VOLTAGES ,
        .mode = LDO_STANDBY ,
    },

    {   
        .reg_addr=      BCM59055_REG_HV3OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
        .reg_addr_volt= BCM59055_REG_HVLDO3CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_LDO_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_ldo_uv_table,
        .num_voltages = N_LDO_VOLTAGES ,
        .mode = LDO_STANDBY ,
    },

    {   
        .reg_addr=      BCM59055_REG_HV4OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
        .reg_addr_volt= BCM59055_REG_HVLDO4CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_LDO_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_ldo_uv_table,
        .num_voltages = N_LDO_VOLTAGES ,
        .mode = LDO_STANDBY ,
    },

    {   
        .reg_addr=      BCM59055_REG_HV5OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
        .reg_addr_volt= BCM59055_REG_HVLDO5CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_LDO_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_ldo_uv_table,
        .num_voltages = N_LDO_VOLTAGES ,
        .mode = LDO_STANDBY ,
    },

    {   
        .reg_addr=      BCM59055_REG_HV6OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
        .reg_addr_volt= BCM59055_REG_HVLDO6CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_LDO_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_ldo_uv_table,
        .num_voltages = N_LDO_VOLTAGES ,
        .mode = LDO_STANDBY ,
    },

    {   
        .reg_addr=      BCM59055_REG_HV7OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
        .reg_addr_volt= BCM59055_REG_HVLDO7CTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_LDO_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_ldo_uv_table,
        .num_voltages = N_LDO_VOLTAGES ,
        .mode = LDO_STANDBY ,
    },

    {   
        .reg_addr=      BCM59055_REG_SIMOPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
        .reg_addr_volt= BCM59055_REG_SIMLDOCTRL,     /* BCM59035_REGULATOR_HVLDO1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_LDO_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_ldo_uv_table,
        .num_voltages = N_LDO_VOLTAGES ,
        .mode = LDO_STANDBY ,
    },

    {  
        .reg_addr =      BCM59055_REG_CSROPMODCTRL,  /* BCM59055_REG_CSROPMODCTRL                          */
        .reg_addr_volt = BCM59055_REG_CSRCTRL1,      /* BCM59055_REG_CSRCTRL1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_SR_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_sr_uv_table,
        .num_voltages = N_SR_VOLTAGES,
        .mode = LDO_STANDBY ,
    },  

    {  
        .reg_addr =      BCM59055_REG_IOSROPMODCTRL,  /* BCM59055_REG_IOSROPMODCTRL                          */
        .reg_addr_volt = BCM59055_REG_IOSRCTRL1,      /* BCM59055_REG_IOSRCTRL1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_SR_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_sr_uv_table,
        .num_voltages = N_SR_VOLTAGES,
        .mode = LDO_STANDBY ,
    },  

    {  
        .reg_addr =      BCM59055_REG_SDSROPMODCTRL,  /* BCM59055_REG_SDSROPMODCTRL                          */
        .reg_addr_volt = BCM59055_REG_SDSRCTRL1,      /* BCM59055_REG_SDSRCTRL1                           */
        .en_dis_mask   = EN_DIS_MASK ,
        .en_dis_shift  = EN_DIS_SHIFT ,
        .vout_mask=     VOUT_SR_MASK ,
        .vout_shift=    0,
        .v_table= bcm59055_sr_uv_table,
        .num_voltages = N_SR_VOLTAGES, 
        .mode = LDO_STANDBY ,
    },  

} ;

static int bcm59055_get_voltage(struct regulator_dev *rdev) ;
static int bcm59055_set_voltage(struct regulator_dev *rdev, int min_uv, int max_uv) ;
static int bcm59055_list_voltage(struct regulator_dev *rdev, unsigned index) ;
static unsigned int bcm59055_get_mode (struct regulator_dev *rdev) ;
static int bcm59055_set_mode(struct regulator_dev *rdev, unsigned mode) ;
static int bcm59055_disable(struct regulator_dev *rdev) ;
static int bcm59055_enable(struct regulator_dev *rdev) ;
static int bcm59055_is_enabled(struct regulator_dev *rdev) ;

/** voltage regulator details.  */
struct regulator_ops bcm590xxldo_ops = {
    .list_voltage = bcm59055_list_voltage,
    .enable       = bcm59055_enable,
    .disable      = bcm59055_disable,
    .is_enabled   = bcm59055_is_enabled,
    .get_mode     = bcm59055_get_mode ,
    .set_mode     = bcm59055_set_mode,
    .set_voltage  = bcm59055_set_voltage, 
    .get_voltage  = bcm59055_get_voltage,
};

static unsigned int bcm59055_ldo_or_sr(struct regulator_dev *rdev )
{
    if ( rdev_get_id(rdev) > BCM59055_SIMLDO ) 
        return BCM590XX_SR ; 
    else  
        return BCM590XX_LDO ; 
}

/* @is_enabled: Return 1 if the regulator is enabled, 0 if not.
 *		May also return negative errno.
*/
static int bcm59055_is_enabled(struct regulator_dev *rdev)
{
    struct bcm590xx_reg_info  *info = &bcm59055_register_info[0] + rdev_get_id(rdev) ;
    unsigned int ldo_or_sr = bcm59055_ldo_or_sr(rdev) ;
    int retval = 0 ;
    int rc = bcm590xx_reg_read(SLAVE_ID0, info->reg_addr) ;

    if (rc < 0) {
        printk(KERN_ERR "%s reg_read failed on register 0x%x with return value %d \n", __func__, info->reg_addr,rc );
        return rc;
    }

    rc = ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ;

    if ( ldo_or_sr == BCM590XX_LDO ) {
        if ( rc < LDO_OFF )
            retval = 1 ; 
        else 
            retval = 0 ; 
    }
    else if ( ldo_or_sr == BCM590XX_SR ) {
        if ( rc == LDO_OFF )
            retval = 0 ; 
        else  
            retval = 1 ; 
    }
    return retval ; 
}

/* @enable: Configure the regulator as enabled. */
static int bcm59055_enable(struct regulator_dev *rdev)
{
    struct bcm590xx_reg_info  *info = &bcm59055_register_info[0] + rdev_get_id(rdev) ;
    unsigned int ldo_or_sr = bcm59055_ldo_or_sr(rdev) ;
    int retval = 0 ;	

    int rc = bcm590xx_reg_read(SLAVE_ID0, info->reg_addr) ;
    if (rc < 0) {
        printk(KERN_ERR "%s reg_read failed on register 0x%x with return value %d \n", __func__, info->reg_addr,rc );
        return rc;
    }

    // In case of LDO always enable in LPM (1). 
    // This is because LDOs can operating in LPM consume less power, and are enough for operations.
    // In case of SR always enable in NM (0). 
    // This is because all switchers are supposed to operate in normal mode, otherwise it might cause problems to chip.
    if ( ldo_or_sr == BCM590XX_LDO ) {
        if ( ( ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ) < LDO_OFF )
            retval = 1 ;  // Already enabled.
        else { 
            rc = ( rc & ~(info->en_dis_mask << info->en_dis_shift) ) | ( LDO_STANDBY << info->en_dis_shift ) ;
            retval = bcm590xx_reg_write(SLAVE_ID0, info->reg_addr, rc ) ;
        } 
    }
    else if ( ldo_or_sr == BCM590XX_SR ) {
        if ( ( ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ) != LDO_OFF )
            retval = 1 ;  // Already enabled.
        else { 
            rc = ( rc & ~(info->en_dis_mask << info->en_dis_shift) ) | ( LDO_NORMAL << info->en_dis_shift ) ;
            retval = bcm590xx_reg_write(SLAVE_ID0, info->reg_addr, rc ) ;
        } 
    }
    return retval ;
}

/* @disable: Configure the regulator as disabled. */
static int bcm59055_disable(struct regulator_dev *rdev)
{
    struct bcm590xx_reg_info  *info = &bcm59055_register_info[0] + rdev_get_id(rdev) ; 

    int rc = bcm590xx_reg_read(SLAVE_ID0, info->reg_addr) ;

    if (rc < 0) {
        printk(KERN_ERR "%s reg_read failed on register 0x%x with return value %d \n", __func__, info->reg_addr,rc );
        return rc;
    }

    if ( ( ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ) == LDO_OFF ) { 
        return 1 ;  // Already disabled.
    }  
    else { 
        rc = ( rc & ~(info->en_dis_mask << info->en_dis_shift) ) | ( LDO_OFF << info->en_dis_shift ) ;
        return ( bcm590xx_reg_write(SLAVE_ID0, info->reg_addr, rc ) ) ;
    } 
}

/* @get_mode: Get the configured operating mode for the regulator. */
/* Modes supported are REGULATOR_MODE_NORMAL, and REGULATOR_MODE_STANDBY. */
/* This function is called as part of regulator_set_mode(). 
 * When set mode is done from command line ( cd /sys/class/regulator/regulator.3/reg-virt* , echo normal > mode ) 
 * it calls regulator_set_mode. regulator_set_mode calls bcm59055_get_mode() to get present mode.
 * If mode is same, no action is performed. If mode is different, then bcm59055_set_mode() is called to set 
 * desired mode. 
 * */ 
static unsigned int bcm59055_get_mode(struct regulator_dev *rdev)
{
    struct bcm590xx_reg_info  *info = &bcm59055_register_info[0] + rdev_get_id(rdev) ;  
    int rc = bcm590xx_reg_read(SLAVE_ID0,info->reg_addr) ;

    if (rc < 0) {
        printk(KERN_ERR "%s reg_read failed on register 0x%x with return value %d \n",__func__,  info->reg_addr,rc );
        return rc;
    }

    rc = ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ;

    switch ( rc ) {
        case LDO_NORMAL : 
            return REGULATOR_MODE_NORMAL ;  
        case LDO_STANDBY : 
            return REGULATOR_MODE_STANDBY ; 
        default : 
            return -EINVAL ; 
    }
}

/*  @set_mode: Set the configured operating mode for the regulator. */
static int bcm59055_set_mode(struct regulator_dev *rdev, unsigned mode)
{
    struct bcm590xx_reg_info  *info = &bcm59055_register_info[0] + rdev_get_id(rdev) ;  

    int rc = bcm590xx_reg_read(SLAVE_ID0,info->reg_addr) ;

    if (rc < 0) {
        printk(KERN_ERR "%s reg_read failed on register 0x%x with return value %d \n", __func__, info->reg_addr,rc );
        return rc;
    }

    rc = ( rc & ~(info->en_dis_mask << info->en_dis_shift) ) ;
    switch ( mode ) {
        case REGULATOR_MODE_NORMAL :  
            rc = rc | ( LDO_NORMAL << info->en_dis_shift ) ; 
            break ; 
        case REGULATOR_MODE_STANDBY : 
            rc = rc | ( LDO_STANDBY << info->en_dis_shift ) ; 
            break ; 
        default : 
            return -EINVAL ;
    }

    return ( bcm590xx_reg_write(SLAVE_ID0,info->reg_addr, rc ) ) ;
}

/* @list_voltage: Return one of the supported voltages, in microvolts; zero
 *	if the selector indicates a voltage that is unusable on this system;
 *	or negative errno.  Selectors range from zero to one less than
 *	regulator_desc.n_voltages.  Voltages may be reported in any order.
 *	*/

static int bcm59055_list_voltage(struct regulator_dev *rdev, unsigned index)
{
    struct bcm590xx_reg_info  *info = &bcm59055_register_info[0] + rdev_get_id(rdev) ; 
    return ( info->v_table[index] ) ;
}

static int bcm590xxreg_get_voltage_reg_addr(struct bcm590xx_reg_info  *info, unsigned int mode, unsigned int *addr)
{
    switch ( mode ) {
        case REGULATOR_MODE_NORMAL  : 
            *addr = info->reg_addr_volt ; 
            break ; 
        case REGULATOR_MODE_STANDBY : 
            *addr = info->reg_addr_volt + 1 ; 
            break ; 
        default : 
            return -EINVAL ; // Other case for MODE val are not supported.
    }
    return 0 ;
}

/* @set_voltage: Set the voltage for the regulator within the range specified.
 *               The driver should select the voltage closest to min_uV.
 *               */
static int bcm59055_set_voltage(struct regulator_dev *rdev, int min_uv, int max_uv)
{
    struct bcm590xx_reg_info  *info = &bcm59055_register_info[0] + rdev_get_id(rdev) ; 
    unsigned int ldo_or_sr = bcm59055_ldo_or_sr(rdev ) ;
    unsigned int addr = 0 ;
    unsigned int mode = 0 ;
    int	rc;
    int	ret;

    for (rc = 0; rc < info->num_voltages ; rc++) {
        int uv = info->v_table[rc] ;
        if ( ( min_uv <= uv ) && ( uv <= max_uv )  ) {
            if ( ldo_or_sr == BCM590XX_LDO ) { 
                addr = info->reg_addr_volt ; 
            }
            else if ( ldo_or_sr == BCM590XX_SR ) { 
                // To Set voltage, first get mode you are in for case of CSR, IOSR, SDSR.
                mode =  bcm59055_get_mode (rdev) ;
		ret = bcm590xxreg_get_voltage_reg_addr(info, mode, &addr) ;
		if ( ret < 0 ) {
                    printk(KERN_ERR "%s bcm590xxreg_get_voltage_reg_addr failed, with return value %d \n",__func__, ret );
                    return ret ;                
                }
            }

            /* First read the register. */
            ret = bcm590xx_reg_read(SLAVE_ID0,addr) ;
            if (ret < 0) {
                printk(KERN_ERR "%s reg_read failed on register 0x%x with return value %d \n",__func__,  addr,rc );
                return ret;
            }
            rc = ( ret & (~(info->vout_mask)) )  | rc ;
            return ( bcm590xx_reg_write(SLAVE_ID0, addr, rc ) ) ;
        }
    }
    return -EDOM;
}

static int bcm59055_get_voltage(struct regulator_dev *rdev)
{
    struct bcm590xx_reg_info  *info = &bcm59055_register_info[0] + rdev_get_id(rdev) ; 
    unsigned int ldo_or_sr = bcm59055_ldo_or_sr(rdev ) ;
    unsigned int addr = 0 ;
    unsigned int mode = 0 ;
    int rc = 0 ;
    int	ret;

    if ( ldo_or_sr == BCM590XX_LDO ) 
        addr = info->reg_addr_volt ; 
    else if ( ldo_or_sr == BCM590XX_SR ) { 
        // To Set voltage, first get mode you are in for case of CSR, IOSR, SDSR.
        mode =  bcm59055_get_mode (rdev) ;
        ret = bcm590xxreg_get_voltage_reg_addr(info, mode, &addr) ;
        if ( ret < 0 ) {
            printk(KERN_ERR "%s bcm590xxreg_get_voltage_reg_addr failed, with return value %d \n",__func__, ret );
            return ret ;                
        }
    }

    rc = bcm590xx_reg_read(SLAVE_ID0,addr) ;

    if (rc < 0) {
        printk(KERN_ERR "%s reg_read failed on register 0x%x with return value %d \n", __func__, addr,rc );
        return rc;
    }

    rc = ( rc >> info->vout_shift ) & ( info->vout_mask ) ;
    return (info->v_table[rc]) ;
}

static int bcm590xx_regulator_probe(struct platform_device *pdev)
{
    struct regulator_dev *rdev;
    struct regulator_desc    *bcm590xx_info  ;

    bcm590xx_info = &bcm59055_info[0] ;

    if ( bcm590xx_info == NULL ) {
        printk(KERN_ERR "%s bcm590xx_regulator_probe() failed. bcm590xx_info was NULL, not proceeding with regulator_register \n", __func__ ) ; 
        return -1 ;
    }

    rdev = regulator_register( (bcm590xx_info + pdev->id), &pdev->dev, pdev->dev.platform_data, dev_get_drvdata(&pdev->dev));
    if (IS_ERR(rdev)) {
        printk(KERN_ERR "failed to register %s\n", ( bcm590xx_info + pdev->id)->name);
        return PTR_ERR(rdev);
    }

    return 0;
}

static int bcm590xx_regulator_remove(struct platform_device *pdev)
{
    struct regulator_dev *rdev = platform_get_drvdata(pdev);
    regulator_unregister(rdev);
    return 0;
}

/* Called from board_fpga.c */
int bcm590xx_register_regulator(struct bcm590xx *bcm590xx, int reg,
			      struct regulator_init_data *initdata)
{
    struct platform_device *pdev;
    int ret = 0 ;

    pdev = platform_device_alloc("bcm590xx-regulator", reg);
    if (!pdev)
        return -ENOMEM;

    bcm590xx->pmic.pdev[reg] = pdev;

    initdata->driver_data = bcm590xx;

    pdev->dev.platform_data = initdata;
    pdev->dev.parent = bcm590xx->dev;
    platform_set_drvdata(pdev, bcm590xx);

    ret = platform_device_add(pdev);

    if (ret != 0) {
        printk(KERN_ERR  "Failed to register regulator %d: %d\n", reg, ret);
        platform_device_del(pdev);
        bcm590xx->pmic.pdev[reg] = NULL;
    }

    return ret;
}
EXPORT_SYMBOL_GPL(bcm590xx_register_regulator);

static struct platform_driver bcm590xx_regulator_driver = {
    .probe = bcm590xx_regulator_probe,
    .remove = bcm590xx_regulator_remove,
    .driver		= {
        .name	= "bcm590xx-regulator",
    },
};

static int __init bcm590xx_regulator_init(void)
{
    return platform_driver_register(&bcm590xx_regulator_driver);
}
subsys_initcall(bcm590xx_regulator_init);

static void __exit bcm590xx_regulator_exit(void)
{
    platform_driver_unregister(&bcm590xx_regulator_driver);
}
module_exit(bcm590xx_regulator_exit);

/* Module information */
MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("BCM59055 regulator driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bcm590xx-regulator");

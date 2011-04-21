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

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>

#include <linux/mfd/bcm590xx/bcm59055_A0.h>


#define DEBUG_ON

static int bcm590xxldo_get_voltage(struct regulator_dev *rdev) ;
static int bcm590xxldo_set_voltage(struct regulator_dev *rdev, int min_uv, int max_uv) ;
static int bcm590xxldo_list_voltage(struct regulator_dev *rdev, unsigned index) ;
static unsigned int bcm590xxreg_get_mode (struct regulator_dev *rdev) ;
static int bcm590xxreg_set_mode(struct regulator_dev *rdev, unsigned mode) ;
static int bcm590xxreg_get_status(struct regulator_dev *rdev) ;
static int bcm590xxreg_disable(struct regulator_dev *rdev) ;
static int bcm590xxreg_enable(struct regulator_dev *rdev) ;
static int bcm590xxreg_is_enabled(struct regulator_dev *rdev) ;

/** voltage regulator details.  */
struct regulator_ops bcm590xxldo_ops = {
	.list_voltage	= bcm590xxldo_list_voltage,
	.enable		= bcm590xxreg_enable,
	.disable	= bcm590xxreg_disable,
	.is_enabled	= bcm590xxreg_is_enabled,
	.get_mode 	= bcm590xxreg_get_mode ,
	.set_mode	= bcm590xxreg_set_mode,
	.get_status	= bcm590xxreg_get_status,
	.set_voltage	= bcm590xxldo_set_voltage,
	.get_voltage	= bcm590xxldo_get_voltage,
};

struct bcm590xx_reg_info *bcm590xx_register_info;
struct regulator_desc    *bcm590xx_info  ;
struct bcm590xx_regulator_init_data *bcm590xx_regl_initdata;


struct bcm590xx_regl_priv {
	struct bcm590xx *bcm590xx;
	int num_regl;
	struct regulator_dev *regl[];
};

static unsigned int bcm590xx_ldo_or_sr(struct regulator_dev *rdev )
{
	printk("inside %s: regl id %d\n", __func__, rdev_get_id(rdev));
    if ( rdev_get_id(rdev) > BCM59055_SIMLDO ) { return BCM590XX_SR ; }
	else { return BCM590XX_LDO ; }
	return -EIO;
}

/* @is_enabled: Return 1 if the regulator is enabled, 0 if not.
 *		May also return negative errno.
*/
static int bcm590xxreg_is_enabled(struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev) ;
	unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev) ;
	int rc;
	printk("Inside %s\n", __func__);
	rc = bcm590xx_reg_read(bcm590xx, info->reg_addr) ;

	if (rc < 0)
	{
		printk("bcm590xxldo_is_enabled : error reading regulator OPmode register.\n");
		return rc;
	}

	rc = ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ;

	if ( ldo_or_sr == BCM590XX_LDO )
	{
		if ( rc < LDO_OFF ) { return 1 ; }
		else { return 0 ; }
	}
	else if ( ldo_or_sr == BCM590XX_SR )
	{
		if ( rc == LDO_OFF ) { return 0 ; }
		else { return 1 ; }
	}
	return -EIO;
}

/* @enable: Configure the regulator as enabled. */
static int bcm590xxreg_enable(struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev) ;
	unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev) ;
	int rc;
	printk("Inside %s\n", __func__);
	rc = bcm590xx_reg_read(bcm590xx, info->reg_addr) ;

	if (rc < 0)
	{
		printk("bcm590xxldo_enable : error reading regulator OPmode register.\n");
		return rc;
	}

	// In case of LDO always enable in LPM (1)
	// In case of SR always enable in NM (0)
	if ( ldo_or_sr == BCM590XX_LDO )
	{
		if ( ( ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ) < LDO_OFF ) { return 1 ; }  // Already enabled.
		else
		{
			// return ( bcm590xx_reg_write(bcm590xx, info->reg_addr, ( ( rc & ~info->en_dis_mask  ) | LDO_STANDBY ) ) ) ;
			rc = ( rc & ~(info->en_dis_mask << info->en_dis_shift) ) | ( LDO_NORMAL << info->en_dis_shift ) ;
			rc |= bcm590xx_regl_initdata[rdev_get_id(rdev)].pm_mode_flag;
			return ( bcm590xx_reg_write(bcm590xx, info->reg_addr, rc ) ) ;
		}
	}
	else if ( ldo_or_sr == BCM590XX_SR )
	{
		if ( ( ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ) != LDO_OFF ) { return 1 ; }  // Already enabled.
		else
		{
			// return ( bcm590xx_reg_write(bcm590xx, info->reg_addr, (  ( rc & ~info->en_dis_mask  ) | LDO_NORMAL ) ) ) ;
			rc = ( rc & ~(info->en_dis_mask << info->en_dis_shift) ) | ( LDO_NORMAL << info->en_dis_shift ) ;
			return ( bcm590xx_reg_write(bcm590xx, info->reg_addr, rc ) ) ;
		}
	}
	return -EIO;
}

/* @disable: Configure the regulator as disabled. */
static int bcm590xxreg_disable(struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
    struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev) ;
	int rc;
	printk("Inside %s\n", __func__);
	rc = bcm590xx_reg_read(bcm590xx, info->reg_addr) ;

	if (rc < 0)
	{
		printk("bcm590xxldo_enable : error reading regulator OPmode register.\n");
		return rc;
	}

	if ( ( ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ) == LDO_OFF ) { return 1 ; }  // Already disabled.
	else
	{
        rc = ( rc & ~(info->en_dis_mask << info->en_dis_shift) ) | ( LDO_OFF << info->en_dis_shift ) ;
        return ( bcm590xx_reg_write(bcm590xx, info->reg_addr, rc ) ) ;
	}
}

/* @get_status: Return actual (not as-configured) status of regulator, as a
 *	REGULATOR_STATUS value (or negative errno)
 */
static int bcm590xxreg_get_status(struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
    struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev) ;
    unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev ) ;
	int rc;
	printk("Inside %s\n", __func__);
	rc = bcm590xx_reg_read(bcm590xx, info->reg_addr) ;

	if (rc < 0)
	{
		printk("bcm590xxldo_is_enabled : error reading regulator OPmode register.\n");
		return rc;
	}

	rc = ( rc >> info->en_dis_shift ) & ( info->en_dis_mask ) ;

    switch ( rc )
	{
        case LDO_NORMAL : { return REGULATOR_STATUS_NORMAL ; } // Normal setting.
		case LDO_STANDBY : { return REGULATOR_STATUS_STANDBY ; }
        case LDO_OFF     : { return REGULATOR_STATUS_OFF ; }
		case LDO_RESERVED_SR_IDLE :
        {
            if ( ldo_or_sr == BCM590XX_SR ) { return REGULATOR_STATUS_IDLE ; }
        }
		default : { return -EINVAL ; }
	}
}

/* @get_mode: Get the configured operating mode for the regulator. */
static unsigned int bcm590xxreg_get_mode (struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
    struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev);

    unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev ) ;
    int rc;
    printk("Inside %s, PM Mod reg 0%x\n", __func__, info->reg_addr);
	rc = bcm590xx_reg_read(bcm590xx,info->reg_addr) ;
	//printk("%s: Pm mode register value for regl id %d is 0x%x\n", __func__, rdev_get_id(rdev), rc);
	if (rc < 0)
	{
		printk("bcm590xxldo_is_enabled : error reading regulator OPmode register.\n");
		return rc;
	}

	// Get last 2 bits after read.
	rc = ( rc >> info->en_dis_shift ) & ( info->en_dis_mask );

    switch ( rc )
	{
        case LDO_NORMAL : { return REGULATOR_MODE_NORMAL ; } // Normal setting.
		case LDO_STANDBY : { return REGULATOR_MODE_STANDBY ; }
        case LDO_OFF     : { return -EINVAL ; }  // Mode is not same as status. So return unknown.
		case LDO_RESERVED_SR_IDLE :
        {
            if ( ldo_or_sr == BCM590XX_SR ) { return REGULATOR_MODE_IDLE ; }
        }
		default : { return -EINVAL ; }
	}
}

/*  @set_mode: Set the configured operating mode for the regulator. */
static int bcm590xxreg_set_mode(struct regulator_dev *rdev, unsigned mode)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
    struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev) ;
    unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev ) ;
	int rc;
	printk("Inside %s\n", __func__);
	rc = bcm590xx_reg_read(bcm590xx,info->reg_addr) ;

	if (rc < 0)
	{
		printk("bcm590xxldo_is_enabled : error reading regulator OPmode register.\n");
		return rc;
	}

	// rc = ( rc >> info->en_dis_shift ) & ( ~info->en_dis_mask ) ;
	// Clear up needed 2 bits after read.
    rc = ( rc & ~(info->en_dis_mask << info->en_dis_shift) ) ;

    switch ( mode )
	{
        case REGULATOR_MODE_NORMAL :  { rc = rc | ( LDO_NORMAL << info->en_dis_shift ) ; break ; } // Normal setting.
        case REGULATOR_MODE_STANDBY : { rc = rc | ( LDO_STANDBY << info->en_dis_shift ) ; break ; } // Standby setting.
        case REGULATOR_MODE_IDLE :
        {
            if ( ldo_or_sr == BCM590XX_SR ) { rc = rc | ( LDO_RESERVED_SR_IDLE << info->en_dis_shift ); }
			else if ( ldo_or_sr == BCM590XX_LDO ) { rc = rc | ( LDO_STANDBY << info->en_dis_shift ) ; }
        }
		default : { return -EINVAL ; }
	}

    return ( bcm590xx_reg_write(bcm590xx,info->reg_addr, rc ) ) ;
}

/* @list_voltage: Return one of the supported voltages, in microvolts; zero
 *	if the selector indicates a voltage that is unusable on this system;
 *	or negative errno.  Selectors range from zero to one less than
 *	regulator_desc.n_voltages.  Voltages may be reported in any order.
 *	*/

static int bcm590xxldo_list_voltage(struct regulator_dev *rdev, unsigned index)
{
    struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev) ;

	return ( info->v_table[index] ) ;
}

/* @set_voltage: Set the voltage for the regulator within the range specified.
 *               The driver should select the voltage closest to min_uV.
 *               */
static int bcm590xxldo_set_voltage(struct regulator_dev *rdev, int min_uv, int max_uv)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
    struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev) ;
    unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev ) ;
	unsigned int addr = 0 ;
	unsigned int mode = 0 ;
	int	rc;
	int	ret;
	printk("Inside %s\n", __func__);
	for (rc = 0; rc < info->num_voltages ; rc++)
	{
        int uv = info->v_table[rc] ;

        if ( ( min_uv <= uv ) && ( uv <= max_uv )  )
		{
            if ( ldo_or_sr == BCM590XX_LDO )
			{
                addr = info->reg_addr_volt ;
			}
			else if ( ldo_or_sr == BCM590XX_SR )
			{
                // To Set voltage, first get mode you are in for case of CSR, IOSR, SDSR.
                mode =  bcm590xxreg_get_mode (rdev) ;

                switch ( mode )
                {
                    case REGULATOR_MODE_NORMAL  : { addr = info->reg_addr_volt ; break ; }     // NM1 setting.
                    case REGULATOR_MODE_STANDBY : { addr = info->reg_addr_volt + 1 ; break ; } // LPM setting.
                    case REGULATOR_MODE_IDLE    : { addr = info->reg_addr_volt + 2 ; break ; } // NM2 setting.
                    default : { return -EINVAL ; }  // Other case for MODE val are not supported.
                }
			}

			/* First read the register. */
	        ret = bcm590xx_reg_read(bcm590xx,addr) ;
            if (ret < 0)
            {
                printk("bcm590xxldo_set_voltage : error reading regulator control register.\n");
                return ret;
            }

			rc = ( ret & (~(info->vout_mask)) )  | rc ;

            return ( bcm590xx_reg_write(bcm590xx, addr, rc ) ) ;
		}
	}
	return -EDOM;
}

static int bcm590xxldo_get_voltage(struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev) ;
    unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev ) ;
	unsigned int addr = 0 ;
	unsigned int mode = 0 ;
	int rc = 0 ;
	printk("Inside %s\n", __func__);
    if ( ldo_or_sr == BCM590XX_LDO )
    {
        addr = info->reg_addr_volt ;
    }
    else if ( ldo_or_sr == BCM590XX_SR )
    {
        // To Set voltage, first get mode you are in for case of CSR, IOSR, SDSR.
        mode =  bcm590xxreg_get_mode (rdev) ;
        switch ( mode )
        {
            case REGULATOR_MODE_NORMAL  : { addr = info->reg_addr_volt; break;}     // NM1 setting.
            case REGULATOR_MODE_STANDBY : { addr = info->reg_addr_volt + 1; break;} // LPM setting.
            case REGULATOR_MODE_IDLE    : { addr = info->reg_addr_volt + 2; break;} // NM2 setting.
            default : { return -EINVAL ; }  // Other case for MODE val are not supported.
        }
    }
	printk("%s: %s address is 0x%x\n", __func__, ((ldo_or_sr==0x10)?"LDO":"Switch"), addr);
	rc = bcm590xx_reg_read(bcm590xx,addr) ;

	if (rc < 0)
	{
		printk("bcm590xxldo_get_voltage : error reading regulator control register.\n");
		return rc;
	}

	rc = ( rc >> info->vout_shift ) & ( info->vout_mask ) ;

	return (info->v_table[rc]) ;
}

static int bcm590xx_regulator_probe(struct platform_device *pdev)
{
	//struct regulator_dev *rdev;
	struct bcm590xx *bcm590xx = dev_get_drvdata(pdev->dev.parent);
	struct bcm590xx_regl_priv *regl_priv;
	int i, num;
	printk("REG: bcm590xx_regulator_probe called - bcm590xx 0x%x\n", (u32)bcm590xx);

	num = bcm590xx_num_regl();
#ifdef DEBUG_ON
	printk("%s: Number of regl %d\n", __func__, num);
#endif
	regl_priv = kzalloc((sizeof(struct bcm590xx_regl_priv) +
					num * sizeof(
						struct regulator_dev *)),
						GFP_KERNEL);
	if (unlikely(!regl_priv)) {
			printk("%s: Could not create regl_priv\n", __func__);
			return -ENOMEM;
	}

	regl_priv->bcm590xx = bcm590xx;
	regl_priv->num_regl = num;

	bcm590xx_regl_initdata = (struct bcm590xx_regulator_init_data *)bcm590xx_get_initdata();
	bcm590xx_register_details((void **)&bcm590xx_register_info);
	bcm590xx_regulator_desc((void **)&bcm590xx_info);

	/* register regulator */
	for (i= 0; i < regl_priv->num_regl; i++) {
#ifdef DEBUG_ON
	printk("%s: %d regl name %s, id %d,\n", __func__, i, bcm590xx_info[i].name, bcm590xx_info[i].id);
#endif
		regl_priv->regl[i] = regulator_register(
			&bcm590xx_info[i],
			&pdev->dev, bcm590xx_regl_initdata[i].initdata,
			regl_priv);
	}
	regulator_has_full_constraints();
	/* register regulator IRQ here */
	return 0;
}

static int bcm590xx_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = platform_get_drvdata(pdev);
	// struct bcm590xx *bcm590xx = rdev_get_drvdata(rdev);

	regulator_unregister(rdev);

	return 0;
}

static struct platform_driver bcm590xx_regulator_driver = {
	.probe = bcm590xx_regulator_probe,
	.remove = __devexit_p(bcm590xx_regulator_remove),
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
MODULE_AUTHOR("Saringni Addepally");
MODULE_DESCRIPTION("BCM590XX voltage and current regulator driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bcm590xx-regulator");

MODULE_DESCRIPTION("BCM590xx regulator driver");
MODULE_LICENSE("GPL");

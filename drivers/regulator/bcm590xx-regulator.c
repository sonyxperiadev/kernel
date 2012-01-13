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

static int bcm590xxldo_get_voltage(struct regulator_dev *rdev);
static int bcm590xxldo_set_voltage(struct regulator_dev *rdev, int min_uv, int max_uv, unsigned *selector);
static int bcm590xxldo_list_voltage(struct regulator_dev *rdev, unsigned index);
static unsigned int bcm590xxreg_get_mode (struct regulator_dev *rdev);
static int bcm590xxreg_set_mode(struct regulator_dev *rdev, unsigned mode);
static int bcm590xxreg_get_status(struct regulator_dev *rdev);
static int bcm590xxreg_disable(struct regulator_dev *rdev);
static int bcm590xxreg_enable(struct regulator_dev *rdev);
static int bcm590xxreg_is_enabled(struct regulator_dev *rdev);

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

struct bcm590xx_reg_info		*bcm590xx_register_info;
struct regulator_desc			*bcm590xx_info;
struct bcm590xx_regulator_init_data	*bcm590xx_regl_initdata;


struct bcm590xx_regl_priv {
	struct bcm590xx *bcm590xx;
	int num_regl;
	struct regulator_dev *regl[];
};

/* @is_enabled: Return 1 if the regulator is enabled, 0 if not.
 *		May also return negative errno.
*/
static int bcm590xxreg_is_enabled(struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev);
	int rc;
	rc = bcm590xx_reg_read(bcm590xx, info->reg_addr);
	pr_debug("%s: ID %d Reg addr 0x%x - value 0x%x\n", __func__, rdev_get_id(rdev), info->reg_addr, rc);

	if (rc < 0) {
		pr_info("bcm590xxldo_is_enabled : error reading regulator OPmode register.\n");
		return rc;
	}

	rc = (rc >> PC2_IS_1_PC1_IS_1) & PM_MODE_MASK;
	if (rc == LDO_OFF)
		return 0;
	else
		return 1;
}

/* @enable: Configure the regulator as enabled. */
static int bcm590xxreg_enable(struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev);
	int rc;
	pr_debug("Inside %s: ID %d\n", __func__, rdev_get_id(rdev));
	/* Clear all PM modes to zero which means ON in all mode
	 * We need to enable the regulator in PC2_IS_1_PC1_IS_1 and
	 * PC2_IS_0_PC1_IS_1 state. Rest modes as BB in Deep Sleep.
	 * values for DSM are taken from Board file */
	rc = 0;
	switch (info->dsm) {
		case BCM590XX_REGL_LPM_IN_DSM:
			rc |= (LDO_STANDBY << PC2_IS_0_PC1_IS_0) |
				(LDO_STANDBY << PC2_IS_1_PC1_IS_0);
			break;
		case BCM590XX_REGL_OFF_IN_DSM:
			rc |= (LDO_OFF << PC2_IS_0_PC1_IS_0) |
				(LDO_OFF << PC2_IS_1_PC1_IS_0);
		case BCM590XX_REGL_ON_IN_DSM:
			/* has been taken care already */
			break;
	}

	return bcm590xx_reg_write(bcm590xx, info->reg_addr, rc);
}

/* @disable: Configure the regulator as disabled. */
static int bcm590xxreg_disable(struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev);
	int rc = 0;
	pr_debug("Inside %s: ID %d\n", __func__, rdev_get_id(rdev));
	/* OFF in all PM Modes */
	rc = (LDO_OFF << PC2_IS_0_PC1_IS_0) |
		(LDO_OFF << PC2_IS_0_PC1_IS_1) |
		(LDO_OFF << PC2_IS_1_PC1_IS_0) |
		(LDO_OFF << PC2_IS_1_PC1_IS_1);

	return  bcm590xx_reg_write(bcm590xx, info->reg_addr, rc);
}

/* @get_status: Return actual (not as-configured) status of regulator, as a
 *	REGULATOR_STATUS value (or negative errno)
 */
static int bcm590xxreg_get_status(struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev);
	unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev_get_id(rdev));
	int rc;
	rc = bcm590xx_reg_read(bcm590xx, info->reg_addr);
	pr_debug("%s: ID %d Reg addr 0x%x - value 0x%x\n", __func__, rdev_get_id(rdev), info->reg_addr, rc);
	if (rc < 0) {
		printk("bcm590xxldo_is_enabled : error reading regulator OPmode register.\n");
		return rc;
	}
	rc = (rc >> PC2_IS_1_PC1_IS_1) & PM_MODE_MASK;

	switch ( rc ) {
		case LDO_NORMAL:
			return REGULATOR_STATUS_NORMAL;
		case LDO_STANDBY:
			return REGULATOR_STATUS_STANDBY;
		case LDO_OFF:
			return REGULATOR_STATUS_OFF;
		case LDO_RESERVED_SR_FAST:
			if (ldo_or_sr == BCM590XX_SR)
				return REGULATOR_STATUS_FAST;
	}
	return -EINVAL;
}

/* @get_mode: Get the configured operating mode for the regulator. */
static unsigned int bcm590xxreg_get_mode (struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev);

	unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev_get_id(rdev));
	int rc;
	rc = bcm590xx_reg_read(bcm590xx,info->reg_addr);
	pr_debug("%s: ID %d Reg addr 0x%x - value 0x%x\n", __func__, rdev_get_id(rdev), info->reg_addr, rc);
	if (rc < 0) {
		pr_info("bcm590xxldo_is_enabled : error reading regulator OPmode register.\n");
		return rc;
	}
	// Get last 2 bits after read.
	rc = (rc >> PC2_IS_1_PC1_IS_1) & PM_MODE_MASK;

	switch ( rc ) {
		case LDO_NORMAL:
			return REGULATOR_MODE_NORMAL;
		case LDO_STANDBY:
			return REGULATOR_MODE_STANDBY;
		case LDO_OFF:
			return REGULATOR_MODE_IDLE;
		case LDO_RESERVED_SR_FAST:
			if (ldo_or_sr == BCM590XX_SR)
				return REGULATOR_MODE_FAST;
	}
	return -EINVAL;
}

/*  @set_mode: Set the configured operating mode for the regulator. */
static int bcm590xxreg_set_mode(struct regulator_dev *rdev, unsigned mode)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev);
	unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev_get_id(rdev));
	int rc;
	pr_debug("Inside %s: ID %d\n", __func__, rdev_get_id(rdev));
	rc = bcm590xx_reg_read(bcm590xx,info->reg_addr);
	if (rc < 0) {
		pr_info("bcm590xxldo_is_enabled : error reading regulator OPmode register.\n");
		return rc;
	}
	// Clear up needed 2 bits after read.
	rc &= ~((PM_MODE_MASK << PC2_IS_1_PC1_IS_1) | (PM_MODE_MASK <<PC2_IS_0_PC1_IS_1));

	switch (mode) {
		case REGULATOR_MODE_NORMAL:
			rc |= (LDO_NORMAL << PC2_IS_1_PC1_IS_1) |
				(LDO_NORMAL << PC2_IS_0_PC1_IS_1);
			break;
		case REGULATOR_MODE_STANDBY :
			rc |= (LDO_STANDBY << PC2_IS_1_PC1_IS_1) |
				(LDO_STANDBY << PC2_IS_0_PC1_IS_1);
			break;
		case REGULATOR_MODE_IDLE:
			rc |= (LDO_OFF << PC2_IS_1_PC1_IS_1) |
				(LDO_OFF << PC2_IS_0_PC1_IS_1);
			break;
		case REGULATOR_MODE_FAST:
			if (ldo_or_sr == BCM590XX_SR)
				rc |= (LDO_RESERVED_SR_FAST << PC2_IS_1_PC1_IS_1) |
					(LDO_RESERVED_SR_FAST << PC2_IS_0_PC1_IS_1);
			break;
		default : { return -EINVAL; }
	}
	return bcm590xx_reg_write(bcm590xx,info->reg_addr, rc);
}

/* @list_voltage: Return one of the supported voltages, in microvolts; zero
 *	if the selector indicates a voltage that is unusable on this system;
 *	or negative errno.  Selectors range from zero to one less than
 *	regulator_desc.n_voltages.  Voltages may be reported in any order.
 *	*/

static int bcm590xxldo_list_voltage(struct regulator_dev *rdev, unsigned index)
{
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev);
	return ( info->v_table[index] );
}

/* @set_voltage: Set the voltage for the regulator within the range specified.
 *               The driver should select the voltage closest to min_uV.
 *               */
static int bcm590xxldo_set_voltage(struct regulator_dev *rdev, int min_uv, int max_uv, unsigned *selector)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev);
	unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev_get_id(rdev));
	unsigned int addr = 0;
	unsigned int mode = 0;
	int	rc, uv=0;
	int	ret;
	pr_debug("Inside %s: ID %d\n", __func__, rdev_get_id(rdev));
	/* Find the proper voltage between MAX and MIN voltage */
	for (rc = 0; rc < info->num_voltages; rc++) {
		uv = info->v_table[rc];
		if ((min_uv <= uv) && (uv <= max_uv))
			break;
	}
	pr_debug("%s: selected uv is %d at rc %d\n", __func__, uv, rc);
	if (rc == info->num_voltages) {
		pr_info("%s: No Suitable voltage found\n", __func__);
		return -EINVAL;
	}
	if (ldo_or_sr == BCM590XX_LDO)
		addr = info->reg_addr_volt;
	if (ldo_or_sr == BCM590XX_SR) {
		// To Set voltage, first get mode you are in for case of CSR, IOSR, SDSR.
		mode =  bcm590xxreg_get_mode(rdev);

		switch (mode) {
			case REGULATOR_MODE_NORMAL:
				addr = info->reg_addr_volt;
				break;
			case REGULATOR_MODE_STANDBY:
				addr = info->reg_addr_volt + 1;
				break;
			case REGULATOR_MODE_FAST:
				addr = info->reg_addr_volt + 2;
				break;
			default:
				return -EINVAL;
		}
	}
	/* First read the register. */
	ret = bcm590xx_reg_read(bcm590xx, addr);
	if (ret < 0) {
		pr_info("bcm590xxldo_set_voltage : error reading regulator control register.\n");
		return ret;
	}

	*selector = rc;

	rc = (ret & ~info->vout_mask) | (rc << info->vout_shift);
	return bcm590xx_reg_write(bcm590xx, addr, rc);
}

static int bcm590xxldo_get_voltage(struct regulator_dev *rdev)
{
	struct bcm590xx_regl_priv *priv = rdev_get_drvdata(rdev);
	struct bcm590xx	*bcm590xx = priv->bcm590xx;
	struct bcm590xx_reg_info  *info = bcm590xx_register_info + rdev_get_id(rdev);
	unsigned int ldo_or_sr = bcm590xx_ldo_or_sr(rdev_get_id(rdev));
	unsigned int addr = 0;
	unsigned int mode = 0;
	int rc = 0;
	pr_debug("Inside %s: ID %d\n", __func__, rdev_get_id(rdev));
	if (ldo_or_sr == BCM590XX_LDO)
		addr = info->reg_addr_volt;

	if ( ldo_or_sr == BCM590XX_SR ) {
		// To Set voltage, first get mode you are in for case of CSR, IOSR, SDSR.
		mode =  bcm590xxreg_get_mode(rdev);
		switch (mode) {
			case REGULATOR_MODE_NORMAL:
				addr = info->reg_addr_volt;
				break;
			case REGULATOR_MODE_STANDBY:
				addr = info->reg_addr_volt + 1;
				break;
			case REGULATOR_MODE_FAST:
				addr = info->reg_addr_volt + 2;
				break;
			default:
				return -EINVAL;
		}
	}
	rc = bcm590xx_reg_read(bcm590xx,addr);
	if (rc < 0) {
		pr_info("bcm590xxldo_get_voltage : error reading regulator control register.\n");
		return rc;
	}
	rc = (rc >> info->vout_shift) & (info->vout_mask);
	return (info->v_table[rc]);
}

static int bcm590xx_regulator_probe(struct platform_device *pdev)
{
	//struct regulator_dev *rdev;
	struct bcm590xx *bcm590xx = dev_get_drvdata(pdev->dev.parent);
	struct bcm590xx_regl_priv *regl_priv;
	struct bcm590xx_regulator_pdata *regulator = bcm590xx->pdata->regl_pdata;
	int i, ret=0;
	pr_info("%s: - bcm590xx 0x%x\n", __func__, (u32)bcm590xx);

	bcm590xx_register_details((void **)&bcm590xx_register_info);
	bcm590xx_regulator_desc((void **)&bcm590xx_info);
	bcm590xx_regl_initdata = regulator->init;

	/* Set default values for all regulators */
	for (i = 0; i < BCM590XX_MAX_REGULATOR; i++)
		ret |= bcm590xx_reg_write(bcm590xx,
				bcm590xx_register_info[i].reg_addr,
				regulator->default_pmmode[i]);
	if (ret) {
		pr_info("%s: Failed to set default mode for regulators\n", __func__);
		return -EIO;
	}

	regl_priv =(struct bcm590xx_regl_priv *) kzalloc((sizeof(struct bcm590xx_regl_priv) +
				regulator->num_regulator * sizeof(
					struct regulator_dev *)),
			GFP_KERNEL);
	if (!regl_priv) {
		pr_info("%s: Could not create regl_priv\n", __func__);
		return -ENOMEM;
	}

	regl_priv->bcm590xx = bcm590xx;
	regl_priv->num_regl = regulator->num_regulator;
	pr_info("%s: Set CSR VOLTAGE NM %d, LPM %d, NM2 %d\n", __func__,
				regulator->csr_nm_volt, regulator->csr_lpm_volt, regulator->csr_turbo_volt);
	if (set_csr_volt(regulator->csr_nm_volt, regulator->csr_lpm_volt,
					regulator->csr_turbo_volt, bcm590xx)) {
					kfree(regl_priv);
					return -EIO;
	}
	/* register regulator */
	for (i= 0; i < regl_priv->num_regl; i++) {
		int index = bcm590xx_regl_initdata[i].regulator;
		bcm590xx_register_info[index].dsm = bcm590xx_regl_initdata[i].dsm;
		pr_debug("%s: %d regl name %s, id %d,Index %d, dsm %d\n", __func__, i,
				bcm590xx_info[index].name, bcm590xx_info[index].id,
				index, bcm590xx_regl_initdata[i].dsm);
		regl_priv->regl[i] = regulator_register(
				&bcm590xx_info[index],
				&pdev->dev, bcm590xx_regl_initdata[i].initdata,
				regl_priv);
	}
	regulator_has_full_constraints();
	return 0;
}

static int bcm590xx_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = platform_get_drvdata(pdev);
	regulator_unregister(rdev);
	return 0;
}

static struct platform_driver bcm590xx_regulator_driver = {
	.probe	= bcm590xx_regulator_probe,
	.remove	= __devexit_p(bcm590xx_regulator_remove),
	.driver	= {
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
